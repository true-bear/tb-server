#include "../pch.h"
#include "logic.h"
#include "logic_chat.h"
#include <ranges>

import util.packet;
import util.conf;

LogicManager::LogicManager() = default;
LogicManager::~LogicManager() 
{ 
    Stop(); 
}

std::size_t LogicManager::ShardIndex(const std::uint64_t sessionId) const noexcept
{
    std::size_t k = mShards.size();
    return static_cast<int>(sessionId % k);
}

bool LogicManager::Init(SessionGetFunc getSession, const int threadCount)
{
    mGetSessionFunc = std::move(getSession);


    mDispatcher.Register(static_cast<size_t>(PacketType::CHAT), &ProcessChat);

    mShards.resize(threadCount);

    for (auto i : std::views::iota(0, threadCount))
    {
        auto q = std::make_unique<PacketQueueT>();
        auto th = std::make_unique<LogicThread>(
            std::format("LogicThread-{}", i),
            mGetSessionFunc,
            mDispatcher,
            *q,
			ThreadType::Logic
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
        if (s.thread)
            s.thread->Start();
    }
}

void LogicManager::Stop() 
{ 
    for (auto& s : mShards) 
    {
        if (s.thread) 
            s.thread->Stop();
    } 
}
bool LogicManager::DispatchPacket(std::uint64_t sessionId, std::span<const std::byte> frame) noexcept
{
    std::uint16_t type{};
    std::uint32_t bodyLen{};

    if (!packet_util::ReadHeader(frame, type, bodyLen))
        return false;

    if (bodyLen == 0 || bodyLen > NetDefaults::MAX_PACKET_SIZE)
        return false;

    if (packet_util::kHeaderSize + static_cast<std::size_t>(bodyLen) > frame.size())
        return false;

    auto& shard = mShards[ShardIndex(sessionId)];


    PacketNode* n = new (std::nothrow) PacketNode{};
    if (!n) 
        return false;

    n->sessionId = sessionId;
    n->type = type;
    n->size = bodyLen;
    n->data = static_cast<std::byte*>(::operator new(bodyLen, std::nothrow));

    if (!n->data) 
    { 
        ::operator delete(n->data);
        delete n;
        return false; 
    }

    std::memcpy(n->data, frame.data() + packet_util::kHeaderSize, bodyLen);


    if (!shard.queue->push(n))
    {
        ::operator delete(n->data);
        delete n;        
        return false;
    }

    return true;
}