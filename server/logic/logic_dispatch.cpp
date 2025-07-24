#include "pch.h"
#include "logic_dispatch.h"


void LogicDispatch::Register(size_t packetType, RecvFunc func)
{
    mRecvFuncMap[packetType] = std::move(func);
}

void LogicDispatch::Dispatch(size_t packetType, ClientSession* session, std::span<const std::byte> payload)
{
    if (auto iter = mRecvFuncMap.find(packetType); iter != mRecvFuncMap.end())
    {
        iter->second(session, payload);
    }
}
