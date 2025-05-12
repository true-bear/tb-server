#pragma once
#include "pch.h"
#include "message.pb.h"
#include "clientSession.h"


void ProcessChat(ClientSession* session, const char* data, uint16_t size)
{
    if (!session)
    {
        LOG_ERR("ProcessChat", "session nullptr");
        return;
    }

    ChatPacket chatPacket;
    if (!chatPacket.ParseFromArray(data, size))
    {
        LOG_ERR("ProcessChat", "uid:{} size:{} parse failed", session->GetUniqueId(), size);
        return;
    }

    std::vector<char> serializedData(chatPacket.ByteSizeLong());
    if (!chatPacket.SerializeToArray(serializedData.data(), static_cast<int>(serializedData.size())))
    {
        LOG_ERR("ProcessChat", "Serialize failed for uid:{}", session->GetUniqueId());
        return;
    }

    if (!session->SendPacket(serializedData.data(), static_cast<uint16_t>(serializedData.size())))
    {
        LOG_ERR("ProcessChat", "SendPacket failed uid:{} size:{}", session->GetUniqueId(), serializedData.size());
    }

    LOG_INFO("Chat", "Send uid:{} msg:'{}'", session->GetUniqueId(), chatPacket.message());
}
