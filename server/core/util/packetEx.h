#pragma once
#include "pch.h"

struct PacketEx {
    int mUID{ 0 };
    uint16_t mSize{ 0 };
    std::vector<char> mData;

    PacketEx() = default;

    PacketEx(const int uid, uint16_t size, const char* buf)
        : mUID(uid), mSize(size), mData(buf, buf + size) {
    }

    PacketEx(const PacketEx&) = delete;
    PacketEx& operator=(const PacketEx&) = delete;

    PacketEx(PacketEx&&) = default;
    PacketEx& operator=(PacketEx&&) = default;
};

