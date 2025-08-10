#include "../pch.h"
#include "logic_dispatch.h"


void LogicDispatch::Register(size_t packetType, RecvFunc fn) 
{
    mRecvFuncMap[packetType] = fn;
}

void LogicDispatch::Dispatch(size_t packetType, Session* session, const std::byte* data, size_t size) const
{
    if (auto it = mRecvFuncMap.find(packetType); it != mRecvFuncMap.end())
        it->second(session, data, size);
}
