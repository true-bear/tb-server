#pragma once
#include "pch.h"


#pragma pack(push, 1)
struct PacketHead
{
    uint32_t length;
    uint32_t type;
    uint32_t packet_id;
};
#pragma pack(pop)

struct PacketEx
{
    int sessionId;
    uint16_t type;
    uint32_t packetId;
    std::vector<char> payload;

    PacketEx(int sid, uint16_t t, uint32_t pid, const char* data, uint16_t len)
        : sessionId(sid), type(t), packetId(pid), payload(data, data + len)
    {
    }
};

