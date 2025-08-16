#pragma once
#include "../common_types.h"
#include "../pch.h"
#include <format>
#include <vector>

import iocp.session;

inline void ProcessChat(Session* gwSession, const std::byte* data, size_t size)
{
    if (!gwSession) return;

    ChatPacket chat;
    if (!chat.ParseFromArray(data, static_cast<int>(size)))
        return;

    chat.set_message("show me the money");

    std::string payload;
    if (!chat.SerializeToString(&payload))
        return;

    // 하드코딩 지워야함
    const uint32_t cidN = htonl(999);

    std::string relay;
    relay.resize(sizeof(uint32_t) + payload.size());
    std::memcpy(relay.data(), &cidN, sizeof(uint32_t));
    std::memcpy(relay.data() + sizeof(uint32_t), payload.data(), payload.size());

    const auto bytes = std::span<const std::byte>(
        reinterpret_cast<const std::byte*>(relay.data()),
        relay.size());

    (void)gwSession->SendPacket(bytes);

    std::cout << std::format("send success : size: {} message: {}\n", payload.size(), chat.message());
}