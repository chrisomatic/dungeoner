#pragma once

#include "3dmath.h"

#define MAX_CLIENTS 8
#define MAX_PACKET_DATA_SIZE 1024

typedef enum
{
    PACKET_TYPE_INIT,
    PACKET_TYPE_CONNECT_REQUEST,
    PACKET_TYPE_CONNECT_CHALLENGE,
    PACKET_TYPE_CONNECT_CHALLENGE_RESP,
    PACKET_TYPE_CONNECT_ACCEPTED,
    PACKET_TYPE_CONNECT_REJECTED,
    PACKET_TYPE_DISCONNECT,
    PACKET_TYPE_PING,
    PACKET_TYPE_UPDATE,
    PACKET_TYPE_STATE,
    PACKET_TYPE_ERROR,
} PacketType;

typedef enum
{
    DISCONNECTED,
    SENDING_CONNECTION_REQUEST,
    SENDING_CHALLENGE_RESPONSE,
    CONNECTED,
} ConnectionState;

typedef enum
{
    CONNECT_REJECT_REASON_SERVER_FULL,
    CONNECT_REJECT_REASON_INVALID_PACKET,
    CONNECT_REJECT_REASON_FAILED_CHALLENGE,
} ConnectionRejectionReason;

typedef enum
{
    PACKET_ERROR_NONE,
    PACKET_ERROR_BAD_FORMAT,
    PACKET_ERROR_INVALID,
} PacketError;

typedef struct
{
    uint32_t game_id;
    uint16_t id;
    PacketType type;
} __attribute__((__packed__)) PacketHeader;

typedef struct
{
    PacketHeader hdr;

    uint32_t data_len;
    uint8_t  data[MAX_PACKET_DATA_SIZE];
} __attribute__((__packed__)) Packet;

extern char* server_ip_address;

// Server
int net_server_start();

// Client
bool net_client_init();
bool net_client_connect();
bool net_client_set_server_ip(char* address);
bool net_client_data_waiting();
int net_client_send(uint8_t* data, uint32_t len);
int net_client_recv(Packet* pkt, bool* is_latest);
void net_client_deinit();
