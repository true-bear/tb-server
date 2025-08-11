#pragma once
#include <unordered_map>
#include "common_types.h"


template<class F>
concept RecvHandler =
    requires(F f, Session * s, const std::byte * d, size_t n) {
        { f(s, d, n) } -> std::same_as<void>;
};


class LogicDispatch 
{
public:
    template<RecvHandler F>
    void Register(size_t type, F fn) 
    {
        mRecvFuncMap[type] = fn; 
    }

    void Dispatch(size_t type, Session* s, const std::byte* d, size_t n) const 
    {
        if (auto it = mRecvFuncMap.find(type); it != mRecvFuncMap.end())
            it->second(s, d, n);
    }
private:
    std::unordered_map<size_t, void(*)(Session*, const std::byte*, size_t)> mRecvFuncMap{};
};
