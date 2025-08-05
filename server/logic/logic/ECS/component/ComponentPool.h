#pragma once
#include "../pch.h"

template<typename T>
class ComponentPool
{
public:
    void Add(uint32_t entityId, const T& component)
    {
        mData[entityId] = component;
    }

    void Remove(uint32_t entityId)
    {
        mData.erase(entityId);
    }

    T* Get(uint32_t entityId)
    {
        auto it = mData.find(entityId);
        if (it != mData.end())
            return &it->second;
        return nullptr;
    }

    bool Has(uint32_t entityId) const
    {
        return mData.find(entityId) != mData.end();
    }

private:
    std::unordered_map<uint32_t, T> mData;
};
