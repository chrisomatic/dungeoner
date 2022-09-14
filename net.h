#pragma once

#include "3dmath.h"

#define MAX_CLIENTS 8
#define MAX_PACKET_DATA_SIZE 1024

typedef enum
{
    PACKET_TYPE_INIT,
    PACKET_TYPE_WORLD_STATE
} PacketType;

typedef struct
{
    uint32_t game_id;
    uint16_t packet_id;
    PacketType type;
    uint16_t ack;
    uint32_t ack_bitfield;
} __attribute__((__packed__)) PacketHeader;

typedef struct
{
    PacketHeader header;

    uint32_t data_len;
    uint8_t  data[MAX_PACKET_DATA_SIZE];
} __attribute__((__packed__)) Packet;

typedef struct
{
    Vector3f position;
    float angle_h;
    float angle_v;
} __attribute__((__packed__)) ClientData;

typedef struct
{
    uint16_t        version;
    uint16_t        num_clients;
    uint16_t        ignore_id;
    ClientData client_data[MAX_CLIENTS];
} WorldState;

extern uint32_t game_id;
extern char* server_ip_address;

// Server
int net_server_start();

// Client
bool net_client_init();
bool net_client_set_server_ip(char* address);
bool net_client_data_waiting();
int net_client_send(uint8_t* data, uint32_t len);
int net_client_recv(Packet* pkt, bool* is_latest);
void net_client_deinit();
