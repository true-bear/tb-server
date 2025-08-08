#include "../pch.h"
#include "logic_dispatch.h"


void LogicDispatch::Register(size_t packetType, RecvFunc func)
{
    mRecvFuncMap.insert_or_assign(packetType, std::move(func));
}

void LogicDispatch::Dispatch(size_t packetType, Session* session, const std::byte* data, size_t size)
{
    if (auto iter = mRecvFuncMap.find(packetType); iter != mRecvFuncMap.end())
    {
        iter->second(session, data, size);
    }
}