#pragma once
#include "../pch.h"

class PacketEx 
{
public:
    PacketEx(int sessionId, std::span<const std::byte> data)
        : mSessionId(sessionId), mData(data.begin(), data.end()) {
    }

    const std::vector<std::byte>& GetData() const { return mData; }
    int GetSessionId() const { return mSessionId; }

private:
    int mSessionId;
    std::vector<std::byte> mData;
};

