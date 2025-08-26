#pragma once
#include "../common_types.h"
#include "../pch.h"
#include <format>
#include <vector>

import iocp.session;
import util.packet;

import <span>;

using Byte = std::byte;

inline void ProcessChat(Session* gwSession, const std::byte* data, const size_t size)
{
    if (!gwSession) 
        return;

    std::span<const Byte> frame{ data, size };

    std::uint32_t clientSid{};
    if (!packet_util::UnWrapPacket(frame, clientSid))
        return;

    ChatPacket chat;
    if (!chat.ParseFromArray(reinterpret_cast<const char*>(frame.data()),
        static_cast<int>(frame.size())))
        return;

    chat.set_message("simple echo");

    std::string payload;
    if (!chat.SerializeToString(&payload))
        return;

    auto relay = packet_util::WrapPacket(clientSid, payload);
    
    if (gwSession->SendPacket({ relay.data(), relay.size() }))
    {
        std::cout << std::format("send success : size: {} message: {}\n", relay.size(), chat.message());
    }
}