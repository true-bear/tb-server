#pragma once
#include "../common_types.h"
#include "../pch.h"
#include <format>
#include <vector>


import iocp.session;

void ProcessChat(Session* session, const std::byte* data, size_t size)
{
    if (!session) return;

    ChatPacket chat;
    if (!chat.ParseFromArray(data, static_cast<int>(size))) return;

    chat.set_message("show me the money ");

    thread_local std::string scratch;
    scratch.clear();
    chat.SerializeToString(&scratch);

    std::span<const std::byte> out{
        reinterpret_cast<const std::byte*>(scratch.data()),
        scratch.size()
    };


    if (!session->SendPacket(out))
    {
        std::cout << "send failed " << std::endl;
        return;
    }
}

