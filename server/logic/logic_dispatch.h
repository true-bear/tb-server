#pragma once
#include "clientSession.h"

using RecvFunc = std::function<void(ClientSession*, std::span<const std::byte>)>;

class LogicDispatch
{
public:
    void Register(size_t packetType, RecvFunc func);
    void Dispatch(size_t packetType, ClientSession* session, std::span<const std::byte> payload);

private:
    std::unordered_map<size_t, RecvFunc> mRecvFuncMap;
};
