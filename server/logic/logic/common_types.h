#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/stack.hpp>

import iocp.session;
using SessionGetFunc = std::function<Session* (int)>;

inline constexpr std::size_t MAX_PACKET_SIZE = 4096;
inline constexpr std::size_t FREE_LIST_CAP = 32768;
inline constexpr std::size_t PACKET_QUEUE_CAP = 8192;

struct alignas(64) PacketNode 
{
    int       sessionId{};
    int       type{};
    uint32_t  size{};
    std::array<std::byte, MAX_PACKET_SIZE> data;
};

using PacketQueueT = boost::lockfree::queue<
    PacketNode*,
    boost::lockfree::capacity<PACKET_QUEUE_CAP>,
    boost::lockfree::fixed_sized<true>
>;

using FreeListT = boost::lockfree::stack<
    PacketNode*,
    boost::lockfree::capacity<FREE_LIST_CAP>,
    boost::lockfree::fixed_sized<true>
>;
