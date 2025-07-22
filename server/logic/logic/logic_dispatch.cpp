#include "pch.h"
#include "logic_dispatch.h"


void LogicDispatch::Register(size_t packetType, RecvFunc func)
{
    mRecvFuncMap[packetType] = std::move(func);
}


void LogicDispatch::Dispatch(size_t packetType, IocpSession* session, const char* data, uint16_t size, uint32_t packetId)
{
    if (auto iter = mRecvFuncMap.find(packetType); iter != mRecvFuncMap.end())
    {
        iter->second(session, data, size, packetId);
    }
    else
    {
        LOG_WARN("Dispatch", "Unhandled packet type: {}", packetType);
    }
}


