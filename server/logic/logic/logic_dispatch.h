#pragma once
#include <unordered_map>
#include "common_types.h"

using RecvFunc = void(*)(Session*, const std::byte*, size_t);

class LogicDispatch 
{
    public:
        void Register(size_t packetType, RecvFunc fn);

        void Dispatch(size_t packetType, Session* session, const std::byte* data, size_t size) const;
    private:
        std::unordered_map<size_t, RecvFunc> mRecvFuncMap;
};
