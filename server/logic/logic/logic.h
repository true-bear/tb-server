// logic/LogicManager.h
#pragma once
#include "../pch.h"
#include "common_types.h"
#include "logic_dispatch.h"
#include "../thread/logicThread.h"

import util.singleton;

class LogicManager : public Singleton<LogicManager>
{
public:
    LogicManager();
    ~LogicManager();

    bool Init(SessionGetFunc getSession, const int threadCount = 1);
    void Start();
    void Stop();

    bool DispatchPacket(const std::uint64_t sessionId, std::span<const std::byte> frame) noexcept;
    std::uint64_t ShardIndex(const std::uint64_t sessionId) const noexcept;

private:
    struct Shard {
        std::unique_ptr<LogicThread> thread;
        std::unique_ptr<PacketQueueT> queue;
    };

    FreeListT mFreeList;
    std::vector<std::unique_ptr<PacketNode[]>> mPoolBlocks;
    std::vector<Shard> mShards;

    LogicDispatch  mDispatcher;
    SessionGetFunc mGetSessionFunc;

    bool InitPools(std::size_t nodeCount);
};
