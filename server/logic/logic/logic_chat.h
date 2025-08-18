#pragma once
#include "../common_types.h"
#include "../pch.h"
#include <format>
#include <vector>

import iocp.session;
import util.packet;

import <span>;
using Byte = std::byte;

inline void ProcessChat(Session* gwSession, const std::byte* data, size_t size) // span¿∏∑Œ πŸ≤‹∞Õ
{
    if (!gwSession) 
        return;

    std::span<const Byte> frame{ data, size };

    std::uint32_t clientSid{};
    if (!UnWrapPacket(frame, clientSid)) 
        return;

    ChatPacket chat;
    if (!chat.ParseFromArray(reinterpret_cast<const char*>(frame.data()),
        static_cast<int>(frame.size())))
        return;

    chat.set_message("show me the money");

    std::string payload;
    if (!chat.SerializeToString(&payload))
        return;

    auto relay = WrapPacket(clientSid, payload);
    gwSession->SendPacket({ relay.data(), relay.size() });

    std::cout << std::format("send success : size: {} message: {}\n",
        payload.size(), chat.message());
}