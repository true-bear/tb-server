#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/stack.hpp>

inline constexpr std::size_t GW_MAX_PACKET = 4096;
inline constexpr std::size_t GW_SEND_QUEUE_CAP = 8192;
inline constexpr std::size_t GW_FREE_LIST_CAP = 16384;

struct alignas(64) GatewaySendNode 
{
    std::uint64_t sessionId{};
    std::uint32_t size{};
    std::array<std::byte, GW_MAX_PACKET> data{};
};

using GatewaySendQueue =
boost::lockfree::queue<GatewaySendNode,
    boost::lockfree::capacity<GW_SEND_QUEUE_CAP>,
    boost::lockfree::fixed_sized<true>>;
