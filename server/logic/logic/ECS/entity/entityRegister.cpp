#pragma once
#include "../pch.h"
#include "entityRegister.h"
#include "../Component/ComponentPool.h"

template<typename T>
class EntityRegister::TypedPool : public IComponentPool, public ComponentPool<T>
{
public:
    void Remove(EntityID id) override { ComponentPool<T>::Remove(id); }
};

inline EntityRegister::EntityRegister()
    : mNextEntityId(1)
{
}

inline EntityID EntityRegister::CreateEntity()
{
    return mNextEntityId++;
}

inline void EntityRegister::DestroyEntity(EntityID id)
{
    for (auto& [type, pool] : mComponentPools)
    {
        auto base = static_cast<IComponentPool*>(pool.get());
        base->Remove(id);
    }
}

template<typename T>
inline void EntityRegister::AddComponent(EntityID id, const T& component)
{
    GetOrCreatePool<T>()->Add(id, component);
}

template<typename T>
inline T* EntityRegister::GetComponent(EntityID id)
{
    auto pool = GetPool<T>();
    return pool ? pool->Get(id) : nullptr;
}

template<typename T>
inline bool EntityRegister::HasComponent(EntityID id)
{
    auto pool = GetPool<T>();
    return pool && pool->Has(id);
}

template<typename T>
inline void EntityRegister::RemoveComponent(EntityID id)
{
    auto pool = GetPool<T>();
    if (pool) pool->Remove(id);
}

template<typename T>
inline typename EntityRegister::TypedPool<T>* EntityRegister::GetOrCreatePool()
{
    std::type_index index(typeid(T));
    if (mComponentPools.find(index) == mComponentPools.end())
    {
        mComponentPools[index] = std::make_shared<TypedPool<T>>();
    }
    return static_cast<TypedPool<T>*>(mComponentPools[index].get());
}

template<typename T>
inline typename EntityRegister::TypedPool<T>* EntityRegister::GetPool()
{
    auto it = mComponentPools.find(std::type_index(typeid(T)));
    if (it != mComponentPools.end())
        return static_cast<TypedPool<T>*>(it->second.get());
    return nullptr;
}
