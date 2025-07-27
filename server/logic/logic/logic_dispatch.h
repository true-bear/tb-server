#pragma once
#include "pch.h"

using RecvFunc = std::function<void(Session*, std::span<const std::byte>)>;

class LogicDispatch
{
public:
    void Register(size_t packetType, RecvFunc func);
    void Dispatch(size_t packetType, Session* session, std::span<const std::byte> payload);

private:
    std::unordered_map<size_t, RecvFunc> mRecvFuncMap;
};
