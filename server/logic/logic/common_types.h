#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <boost/lockfree/queue.hpp>

import iocp.session;
import util.conf;

using SessionGetFunc = std::function<Session* (std::uint64_t)>;

struct alignas(64) PacketNode 
{
    std::uint64_t sessionId{};
    std::uint16_t type{};
    std::uint32_t size{};
    std::byte* data{};
};

using PacketQueueT = boost::lockfree::queue<
    PacketNode*,
    boost::lockfree::capacity<NetDefaults::MAX_PACKET_SIZE>,
    boost::lockfree::fixed_sized<true>
>;