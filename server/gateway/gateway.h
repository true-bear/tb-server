#pragma once
#include <windows.h>
#include <string>
#include <iostream>
#include <format>
#include <span>

#include "proto/message.pb.h"


import core.engine;
import util.conf;
import util.singleton;

using std::byte;
static constexpr size_t RELAY_HDR = 4;

class Gateway  : public Core, public Singleton<Gateway>
{
public:
    bool InitAndConnect(const int sessionCount, const int worker, const int port);

private:
    static std::span<const byte> ToBytes(const std::string& s)
    {
        return std::span<const byte>(reinterpret_cast<const byte*>(s.data()), s.size());
    }

    void Dispatch(const std::uint64_t id, std::span<const byte> frame);

    void HandleFromClient(const std::uint64_t clientSid, std::span<const byte> frame);
    void HandleFromLogic(std::span<const byte> frame);

private:
    unsigned mLogicSid{ 0 };
    std::atomic<unsigned> mLastClientSid{ 0 };
};
