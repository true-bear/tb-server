#pragma once
#include "iocp\clientSession.h"

using RecvFunc = std::function<void(ClientSession*, const char*, uint16_t)>;

class LogicDispatch
{
public:
    void Register(size_t packetType, RecvFunc func);
    void Dispatch(size_t packetType, ClientSession* session, const char* data, uint16_t size);

private:
    std::unordered_map<size_t, RecvFunc> mRecvFuncMap;
};
