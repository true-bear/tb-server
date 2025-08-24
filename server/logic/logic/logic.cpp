#include "../pch.h"
#include "logic.h"
#include "logic_chat.h"

#include <algorithm>
#include <cstring>
#include <thread>
#include <format>
#include <array>
#include <memory>

import util.packet;

LogicManager::LogicManager() = default;
LogicManager::~LogicManager() { Stop(); }

bool LogicManager::InitPools(std::size_t nodeCount)
{
    constexpr std::size_t BLOCK = 4096;

    for (std::size_t left = nodeCount; left > 0; ) 
    {
        const std::size_t take = std::min(left, BLOCK);
        auto block = std::make_unique<PacketNode[]>(take);

        for (std::size_t i = 0; i < take; ++i)
            mFreeList.push(&block[i]);

        mPoolBlocks.emplace_back(std::move(block));
        left -= take;
    }
    return true;
}

std::uint64_t LogicManager::ShardIndex(const std::uint64_t sessionId) const noexcept
{
    std::size_t k = mShards.size();
    return k ? static_cast<std::uint64_t>(static_cast<std::uint64_t>(sessionId) % k) : 0;
}

bool LogicManager::Init(SessionGetFunc getSession, const int threadCount)
{
    mGetSessionFunc = std::move(getSession);

    InitPools(FREE_LIST_CAP);

    mDispatcher.Register(static_cast<size_t>(PacketType::CHAT), &ProcessChat);

    mShards.resize(threadCount);

    for (int i = 0; i < threadCount; ++i) 
    {
        auto q = std::make_unique<PacketQueueT>();
        auto th = std::make_unique<LogicThread>(
            std::format("LogicThread-{}", i),
            mGetSessionFunc,
            mDispatcher,
            *q,
            mFreeList,
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

bool LogicManager::DispatchPacket(const std::uint64_t sessionId, std::span<const std::byte> frame) noexcept
{
    std::uint16_t type{};
    std::uint32_t bodyLen{};

    if (!packet_util::ReadHeader(frame, type, bodyLen))
        return false;

    if (bodyLen == 0 || bodyLen > MAX_PACKET_SIZE)
        return false;

    if (packet_util::kHeaderSize + static_cast<std::size_t>(bodyLen) > frame.size())
        return false;

    PacketNode* node = nullptr;
    if (!mFreeList.pop(node))
        return false;

    node->sessionId = sessionId;
    node->type = static_cast<int>(type);
    node->size = bodyLen;

    std::memcpy(node->data.data(), frame.data() + packet_util::kHeaderSize, bodyLen);

    auto& q = *mShards[ShardIndex(sessionId)].queue;
    if (!q.push(node))
    {
        mFreeList.push(node);
        return false;
    }

    return true;
}