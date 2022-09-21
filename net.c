#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h> 
#include <time.h>
#include <sys/select.h>

#include "socket.h"
#include "timer.h"
#include "net.h"
#include "packet_queue.h"
#include "log.h"

#define GAME_ID 0x98325423

#define TICK_RATE 20.0f
#define PORT 27001

#define MAXIMUM_RTT 1.0f

#define DISCONNECTION_TIMEOUT 10.0f // seconds

typedef struct
{
    int socket;
    uint16_t local_latest_packet_id;
    uint16_t remote_latest_packet_id;
    PacketQueue latest_received_packets;
} NodeInfo;

// Info server stores about a client
typedef struct
{
    Address address;
    ConnectionState state;
    uint16_t remote_latest_packet_id;
    clock_t  time_of_latest_packet;
    uint64_t server_salt;
    uint64_t client_salt;
    ConnectionRejectionReason last_reject_reason;
    PacketError last_packet_error;
} ClientInfo;

struct
{
    Address address;
    NodeInfo info;
    ClientInfo clients[MAX_CLIENTS];
    int num_clients;
} server = {0};

// ---

#define IMAX_BITS(m) ((m)/((m)%255+1) / 255%255*8 + 7-86/((m)%255+12))
#define RAND_MAX_WIDTH IMAX_BITS(RAND_MAX)

_Static_assert((RAND_MAX & (RAND_MAX + 1u)) == 0, "RAND_MAX not a Mersenne number");

static uint64_t rand64(void)
{
    uint64_t r = 0;
    for (int i = 0; i < 64; i += RAND_MAX_WIDTH) {
        r <<= RAND_MAX_WIDTH;
        r ^= (unsigned) rand();
    }
    return r;
}

static Timer server_timer = {0};

static inline int get_packet_size(Packet* pkt)
{
    return (sizeof(pkt->hdr) + pkt->data_len);
}

static inline bool is_packet_id_greater(uint16_t id, uint16_t cmp)
{
    return ((id > cmp) && (id - cmp <= 32768)) || 
           ((id < cmp) && (cmp - id  > 32768));
}

static char* packet_type_to_str(PacketType type)
{
    switch(type)
    {
        case PACKET_TYPE_INIT: return "INIT";
        case PACKET_TYPE_CONNECT_REQUEST: return "CONNECT REQUEST";
        case PACKET_TYPE_CONNECT_CHALLENGE: return "CONNECT CHALLENGE";
        case PACKET_TYPE_CONNECT_CHALLENGE_RESP: return "CONNECT CHALLENGE RESP";
        case PACKET_TYPE_CONNECT_ACCEPTED: return "CONNECT ACCEPTED";
        case PACKET_TYPE_CONNECT_REJECTED: return "CONNECT REJECTED";
        case PACKET_TYPE_DISCONNECT: return "DISCONNECT";
        case PACKET_TYPE_PING: return "PING";
        case PACKET_TYPE_UPDATE: return "UPDATE";
        case PACKET_TYPE_STATE: return "STATE";
        case PACKET_TYPE_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

static void print_packet(Packet* pkt)
{
    LOGN("Game ID:      0x%08x",pkt->hdr.game_id);
    LOGN("Packet ID:    %u",pkt->hdr.id);
    LOGN("Packet Type: %02X",pkt->hdr.type);
    LOGN("Data Len:     %u",pkt->data_len);
    LOGN("Data:");

    char data[3*1024+1] = {0};
    char byte[4] = {0};
    for(int i = 0; i < pkt->data_len; ++i)
    {
        sprintf(byte,"%02X ",pkt->data[i]);
        memcpy(data+(3*i), byte,3);
    }

    LOGN("%s", data);
}

static bool has_data_waiting(int socket)
{
    fd_set readfds;

    //clear the socket set  
    FD_ZERO(&readfds);
    
    //add client socket to set  
    FD_SET(socket, &readfds);

    int activity;

    struct timeval tv = {0};
    activity = select(socket + 1 , &readfds , NULL , NULL , &tv);

    if ((activity < 0) && (errno!=EINTR))
    {
        perror("select error");
        return false;
    }

    bool has_data = FD_ISSET(socket , &readfds);
    return has_data;
}

static int net_send(NodeInfo* node_info, Address* to, Packet* pkt)
{
    int pkt_len = get_packet_size(pkt);
    int sent_bytes = socket_sendto(node_info->socket, to, (uint8_t*)pkt, pkt_len);

    LOGN("[SENT] Packet %d (%u B)",pkt->hdr.id,sent_bytes);
    print_packet(pkt);

    node_info->local_latest_packet_id++;

    return sent_bytes;
}

static int net_recv(NodeInfo* node_info, Address* from, Packet* pkt, bool* is_latest)
{
    int recv_bytes = socket_recvfrom(node_info->socket, from, (uint8_t*)pkt);

    LOGN("[RECV] Packet %d (%u B)",pkt->hdr.id,recv_bytes);
    print_packet(pkt);

    *is_latest = is_packet_id_greater(pkt->hdr.id,node_info->remote_latest_packet_id);
    if(*is_latest)
    {
        node_info->remote_latest_packet_id = pkt->hdr.id;
    }

    //packet_queue_enqueue(&node_info->latest_received_packets,pkt);

    return recv_bytes;
}

static bool validate_packet(Packet* pkt)
{
    bool valid = true;

    valid &= (pkt->hdr.game_id == GAME_ID);
    valid &= (pkt->hdr.type >= PACKET_TYPE_INIT && pkt->hdr.type <= PACKET_TYPE_STATE);

    switch(pkt->hdr.type)
    {
        case PACKET_TYPE_CONNECT_REQUEST:
        case PACKET_TYPE_CONNECT_CHALLENGE_RESP:
            valid &= (pkt->data_len == 1024); // must be padded out to 1024
            break;
        default:
            break;
    }

    return valid;
}

static bool server_get_client(Address* addr, ClientInfo* cli)
{
    for(int i = 0; i < server.num_clients; ++i)
    {
        if(memcmp(&server.clients[i].address, addr, sizeof(Address) == 0))
        {
            // found existing client, exit
            cli = &server.clients[i];
            return false;
        }
    }

    return true;
}

static bool server_assign_new_client(Address* addr, ClientInfo* cli)
{
    // new client
    for(int i = 0; i < server.num_clients; ++i)
    {
        if(server.clients[i].state == DISCONNECTED)
        {
            cli = &server.clients[i];
            cli->state = SENDING_CONNECTION_REQUEST;
            memcpy(&cli->address, &addr, sizeof(Address));
            server.num_clients++;

            LOGN("New Client! [%u.%u.%u.%u:%u], id: %d, (%d/%d)",addr->a,addr->b,addr->c,addr->d,addr->port, i, server.num_clients, MAX_CLIENTS);

            return true;
        }
    }

    LOGN("Server is full and can't accept new clients.");
    return false;
}

static void server_send(PacketType type, ClientInfo* cli)
{

    Packet pkt = {
        .hdr.game_id = GAME_ID,
        .hdr.id = server.info.local_latest_packet_id,
        .hdr.type = type
    };

    switch(type)
    {
        case PACKET_TYPE_INIT:
            break;
        case PACKET_TYPE_CONNECT_CHALLENGE:
        {
            cli->server_salt = rand64();
            memcpy(&pkt.data[0],(uint8_t*)&cli->client_salt,8);
            memcpy(&pkt.data[1],(uint8_t*)&cli->server_salt,8);
            pkt.data_len = 16;

            net_send(&server.info,&cli->address,&pkt);

        }   break;
        case PACKET_TYPE_CONNECT_ACCEPTED:
        {
            pkt.data_len = 0;
            net_send(&server.info,&cli->address,&pkt);
        }   break;
        case PACKET_TYPE_CONNECT_REJECTED:
        {
            pkt.data_len = 1;
            pkt.data[0] = (uint8_t)cli->last_reject_reason;
            net_send(&server.info,&cli->address,&pkt);
        }   break;
        case PACKET_TYPE_PING:
            pkt.data_len = 0;
            net_send(&server.info,&cli->address,&pkt);
            break;
        case PACKET_TYPE_STATE:
            pkt.data_len = 0;
            net_send(&server.info,&cli->address,&pkt);
            break;
        case PACKET_TYPE_ERROR:
            pkt.data_len = 1;
            pkt.data[0] = (uint8_t)cli->last_packet_error;
            net_send(&server.info,&cli->address,&pkt);
            break;
        default:
            break;
    }
}

int net_server_start()
{
    int sock;

    LOGN("Creating socket.");
    socket_create(&sock);

    LOGN("Binding socket %u to any local ip on port %u.", sock, PORT);
    socket_bind(sock, NULL, PORT);
    server.info.socket = sock;

    // set tick rate
    timer_set_fps(&server_timer,TICK_RATE);
    timer_begin(&server_timer);

    for(;;)
    {
        for(;;)
        {
            // Read all pending packets
            bool data_waiting = has_data_waiting(server.info.socket);
            if(!data_waiting)
                break;

            Address from = {0};
            Packet recv_packet = {0};

            bool is_latest;
            int bytes_received = net_recv(&server.info, &from, &recv_packet, &is_latest);

            //print_packet(&recv_packet);

            ClientInfo cli = {0};

            if(!validate_packet(&recv_packet))
            {
                LOGN("Invalid packet!");
                //server_send(PACKET_TYPE_ERROR,&cli);
                timer_delay_us(10); // delay 10 us
                continue;
            }

            bool new_client = server_get_client(&from, &cli);

            if(new_client)
            {
                if(recv_packet.hdr.type == PACKET_TYPE_CONNECT_REQUEST)
                {
                    bool assign_new_client = server_assign_new_client(&from, &cli);
                    if(assign_new_client)
                    {
                        // store salt
                        cli.client_salt = (uint64_t)recv_packet.data;
                        server_send(PACKET_TYPE_CONNECT_CHALLENGE, &cli);
                    }
                }
            }
            else
            {
                // existing client

                // validate salt
                if(recv_packet.hdr.type == PACKET_TYPE_CONNECT_REQUEST)
                {
                    uint64_t pkt_salt = ((uint64_t*)recv_packet.data)[0];
                    server_send(pkt_salt == cli.client_salt ? PACKET_TYPE_CONNECT_ACCEPTED : PACKET_TYPE_CONNECT_REJECTED,&cli);
                }

                is_latest = is_packet_id_greater(recv_packet.hdr.id,cli.remote_latest_packet_id);

                if(is_latest)
                {
                    cli.remote_latest_packet_id = recv_packet.hdr.id;
                    cli.time_of_latest_packet = timer_get_time();
                }
            }

            timer_delay_us(10); // delay 10 us
        }

        // send state packet to all clients
        if(server.num_clients > 0)
        {
            // disconnect any client that hasn't sent a packet in DISCONNECTION_TIMEOUT
            for(int i = 0; i < server.num_clients; ++i)
            {
                double time_elapsed = timer_get_time() - server.clients[i].time_of_latest_packet;

                if(time_elapsed >= DISCONNECTION_TIMEOUT)
                {
                    Address* addr = &server.clients[i].address;
                    LOGN("Client Disconnected! %u.%u.%u.%u:%u",addr->a,addr->b,addr->c,addr->d,addr->port);

                    // Disconnect client
                    memset(&server.clients[i], 0, sizeof(ClientInfo));
                    server.num_clients--;

                    LOGN("Num Clients: %u",server.num_clients);
                }
            }

            // send world state to connected clients...
            // @TODO

            //server.info.local_latest_packet_id++;
        }

        timer_wait_for_frame(&server_timer);
    }
}

struct
{
    Address address;
    NodeInfo info;
    ConnectionState state;
    uint64_t server_salt;
    uint64_t client_salt;
} client = {0};


bool net_client_set_server_ip(char* address)
{
    // example input:
    // 200.100.24.10

    char num_str[3] = {0};
    uint8_t   bytes[4]  = {0};

    uint8_t   num_str_index = 0, byte_index = 0;

    for(int i = 0; i < strlen(address)+1; ++i)
    {
        if(address[i] == '.' || address[i] == '\0')
        {
            bytes[byte_index++] = atoi(num_str);
            memset(num_str,0,3*sizeof(char));
            num_str_index = 0;
            continue;
        }

        num_str[num_str_index++] = address[i];
    }

    server.address.a = bytes[0];
    server.address.b = bytes[1];
    server.address.c = bytes[2];
    server.address.d = bytes[3];

    server.address.port = PORT;

    return true;
}

// client information
bool net_client_init()
{
    int sock;

    LOGN("Creating socket.");
    socket_create(&sock);

    client.info.socket = sock;

    return true;
}

bool net_client_data_waiting()
{
    bool data_waiting = has_data_waiting(client.info.socket);
    return data_waiting;
}

static void client_send(PacketType type)
{
    Packet pkt = {
        .hdr.game_id = GAME_ID,
        .hdr.id = client.info.local_latest_packet_id,
        .hdr.type = PACKET_TYPE_CONNECT_REQUEST
    };

    switch(type)
    {
        case PACKET_TYPE_CONNECT_REQUEST:
        {
            client.client_salt = rand64();
            memcpy(&pkt.data[0],(uint8_t*)&client.client_salt,8);
            pkt.data_len = 8;

            net_send(&client.info,&server.address,&pkt);

        }   break;
        case PACKET_TYPE_CONNECT_CHALLENGE_RESP:
        {
            pkt.data_len = 0;
            net_send(&client.info,&server.address,&pkt);
        }   break;
        case PACKET_TYPE_PING:
            pkt.data_len = 0;
            net_send(&client.info,&server.address,&pkt);
            break;
        case PACKET_TYPE_UPDATE:
            pkt.data_len = 0;
            net_send(&client.info,&server.address,&pkt);
            break;
        default:
            break;
    }
}

bool net_client_connect()
{
    if(client.state != DISCONNECTED)
        return false; // temporary, handle different states in the future

    client_send(PACKET_TYPE_CONNECT_REQUEST);

    for(;;)
    {
        bool data_waiting = net_client_data_waiting();

        if(!data_waiting)
            break;

        Packet srvpkt = {0};
        bool is_latest;
        int recv_bytes = net_client_recv(&srvpkt, &is_latest);

        if(recv_bytes > 0)
        {
            if(srvpkt.hdr.type == PACKET_TYPE_CONNECT_CHALLENGE)
            {
                uint64_t srv_client_salt = ((uint64_t*)&srvpkt.data[0])[0];

                if(srv_client_salt != client.client_salt)
                {
                    LOGN("Server sent client salt (%llu) doesn't match actual client salt (%llu)", srv_client_salt, client.client_salt);
                    return false;
                }

                client.server_salt = ((uint64_t*)&srvpkt.data[8])[0];
                LOGN("Received Connect Challenge. Server Salt: %llu",client.server_salt);
            }
        }

        timer_delay_us(10); // delay 10 us
    }
}

int net_client_send(uint8_t* data, uint32_t len)
{
    Packet pkt = {
        .hdr.game_id = GAME_ID,
        .hdr.id = client.info.local_latest_packet_id,
    };

    memcpy(pkt.data,data,len);
    pkt.data_len = len;


    int sent_bytes = net_send(&client.info, &server.address, &pkt);
    return sent_bytes;
}

int net_client_recv(Packet* pkt, bool* is_latest)
{
    Address from = {0};
    int recv_bytes = net_recv(&client.info, &from, pkt, is_latest);
    return recv_bytes;
}

void net_client_deinit()
{
    socket_close(client.info.socket);
}
