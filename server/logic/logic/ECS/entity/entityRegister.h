#pragma once

#include "pch.h"
#include "entityID.h"

class EntityRegister
{
public:
    EntityRegister();
    EntityID CreateEntity();
    void DestroyEntity(EntityID id);

    template<typename T>
    void AddComponent(EntityID id, const T& component);

    template<typename T>
    T* GetComponent(EntityID id);

    template<typename T>
    bool HasComponent(EntityID id);

    template<typename T>
    void RemoveComponent(EntityID id);

    template<typename... Components>
    std::vector<EntityID> View();

private:
    class IComponentPool
    {
    public:
        virtual void Remove(EntityID id) = 0;
        virtual ~IComponentPool() = default;
    };

    template<typename T>
    class TypedPool;

    template<typename T>
    TypedPool<T>* GetOrCreatePool();

    template<typename T>
    TypedPool<T>* GetPool();

    EntityID mNextEntityId;
    std::unordered_map<std::type_index, std::shared_ptr<IComponentPool>> mComponentPools;
};

// Include the template implementations
