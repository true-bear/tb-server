#pragma once
#include "pch.h"
#include "../pch.h"
#include "message.pb.h"

void ProcessChat(Session* session, std::span<const std::byte> data)
{
    if (!session)
    {
        LOG_ERR("ProcessChat", "session nullptr");
        return;
    }

    ChatPacket chatPacket;
    if (!chatPacket.ParseFromArray(data.data(), static_cast<int>(data.size())))
    {
        LOG_ERR("ProcessChat", "uid:{} size:{} parse failed", session->GetUniqueId(), data.size());
        return;
    }

    const int serializedSize = chatPacket.ByteSizeLong();
    std::vector<std::byte> serializedData(serializedSize);
    if (!chatPacket.SerializeToArray(reinterpret_cast<void*>(serializedData.data()), serializedSize))
    {
        LOG_ERR("ProcessChat", "Serialize failed for uid:{}", session->GetUniqueId());
        return;
    }

    const std::span<const std::byte> serializedSpan = serializedData;
    if (!session->SendPacket(serializedSpan))
    {
        LOG_ERR("ProcessChat", "SendPacket failed uid:{} size:{}", session->GetUniqueId(), serializedData.size());
    }

    LOG_INFO("Chat", "Send uid:{} msg:'{}'", session->GetUniqueId(), chatPacket.message());
}
