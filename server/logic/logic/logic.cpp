//#include "../pch.h"
#include "../logic.h"
#include "logic_chat.h"
#include <algorithm>

//boost::lockfree::queue<PacketEx*> LogicManager::mPacketQueue{ NetDefaults::PACKET_QUEUE_SIZE };

LogicManager::LogicManager()
{
}

LogicManager::~LogicManager()
{
    Stop();

    for (auto& s : mShards) 
    {
        if (!s.queue) continue;
        PacketEx* pkt = nullptr;
        while (s.queue->pop(pkt)) 
            delete pkt;
    }
}

bool LogicManager::Init(SessionGetFunc getSession, const int threadCount)
{
    mGetSessionFunc = std::move(getSession);

    mDispatcher.Register(static_cast<size_t>(PacketType::CHAT), &ProcessChat);

    mShards.resize(threadCount);

    for (int i = 0; i < threadCount; ++i)
    {
        auto q = std::make_unique<boost::lockfree::queue<PacketEx*>>(NetDefaults::PACKET_QUEUE_SIZE);

        auto th = std::make_unique<LogicThread>(
            std::format("LogicThread-{}", i),
            mGetSessionFunc,
            mDispatcher,
            *q
        );

        mShards[i].queue = std::move(q);
        mShards[i].thread = std::move(th);
    }

    return true;
}

void LogicManager::Start()
{
    for (auto& s : mShards) 
    {
        if (s.thread) s.thread->Start();
    }
}

void LogicManager::Stop()
{
    for (auto& s : mShards) 
        if (s.thread) s.thread->Stop();
}

void LogicManager::DisPatchPacket(int sessionId, std::span<const std::byte> data)
{
    PacketEx* pkt = new PacketEx(sessionId, data);
    if (!pkt)
    {
        std::cout << std::format("DisPatchPacket: packet push failed - queue full? sessionId: {}, data size: {}\n", sessionId, data.size());
        return;
    }

    const int idx = ShardIndex(sessionId);
    auto& q = *mShards[idx].queue;

    if (!q.push(pkt)) 
    {
        delete pkt;
        return;
    }

	std::cout << std::format("DisPatchPacket: Recv packet - uID:{} : {}\n", sessionId, data.size());
}

int LogicManager::ShardIndex(int sessionId) const noexcept 
{
    uint32_t uid = static_cast<uint32_t>(sessionId);
    return static_cast<int>(uid % mShards.size());
}
