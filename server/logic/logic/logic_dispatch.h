#pragma once
#include <unordered_map>
#include <functional>
#include <span>

class Session;
using RecvFunc = std::function<void(Session*, const std::byte*, size_t)>;

class LogicDispatch
{
public:
    void Register(size_t packetType, RecvFunc func);
    void Dispatch(size_t packetType, Session* session, const std::byte* data, size_t size);

private:
    std::unordered_map<size_t, RecvFunc> mRecvFuncMap;
};
