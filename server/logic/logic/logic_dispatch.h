#pragma once
#include "pch.h"

class IocpSession;

using RecvFunc = std::function<void(IocpSession*, const char*, uint16_t)>;

class LogicDispatch
{
public:
    void Register(size_t packetType, RecvFunc func);
    void Dispatch(size_t packetType, IocpSession* session, const char* data, uint16_t size);

private:
    std::unordered_map<size_t, RecvFunc> mRecvFuncMap;
};
