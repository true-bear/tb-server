#include "../pch.h"
#include "logic.h"
#include "logic_chat.h"

#include <algorithm>
#include <cstring>
#include <thread>
#include <format>

LogicManager::LogicManager() = default;
LogicManager::~LogicManager() { Stop(); }

bool LogicManager::InitPools(std::size_t nodeCount)
{
    nodeCount = std::min<std::size_t>(nodeCount, FREE_LIST_CAP);
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

int LogicManager::ShardIndex(int sessionId) const noexcept {
    std::size_t k = mShards.size();
    return k ? static_cast<int>(static_cast<uint32_t>(sessionId) % k) : 0;
}

bool LogicManager::Init(SessionGetFunc getSession, int threadCount)
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
            mFreeList
        );
        mShards[i].queue = std::move(q);
        mShards[i].thread = std::move(th);
    }
    return true;
}

void LogicManager::Start() 
{ 
    for (auto& s : mShards) if (s.thread) s.thread->Start(); 
}
void LogicManager::Stop() 
{ 
    for (auto& s : mShards) if (s.thread) s.thread->Stop(); 
}

bool LogicManager::DispatchPacket(int sessionId, std::span<const std::byte> frame) noexcept
{
    if (frame.size() < 6) return false;

    uint16_t typeN = 0;
    uint32_t bodyLenN = 0;

    std::memcpy(&typeN, frame.data() + 0, 2);
    std::memcpy(&bodyLenN, frame.data() + 2, 4);

    const uint16_t type = ntohs(typeN);
    const uint32_t bodyLen = ntohl(bodyLenN);

    if (bodyLen == 0 || bodyLen > MAX_PACKET_SIZE) 
        return false;

    if (6ull + bodyLen > frame.size())             
        return false;

    PacketNode* node = nullptr;
    if (!mFreeList.pop(node)) return false;

    node->sessionId = sessionId;
    node->type = static_cast<int>(type);
    node->size = bodyLen;

    std::memcpy(node->data.data(), frame.data() + 6, bodyLen);

    auto& q = *mShards[ShardIndex(sessionId)].queue;

    if (!q.push(node)) 
        mFreeList.push(node); return false; 
    
    return true;
}