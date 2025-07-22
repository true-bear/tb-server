#include "pch.h"
#include "logic_chat.h"
void ProcessChat(IocpSession* session, const char* data, uint16_t size, uint32_t packetId)
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

    LOG_INFO("Chat", "Recv uid:{} msg:'{}'", session->GetUniqueId(), chatPacket.message());

    const int payloadSize = chatPacket.ByteSizeLong();
    std::vector<char> payload(payloadSize);

    if (!chatPacket.SerializeToArray(payload.data(), payloadSize))
    {
        LOG_ERR("ProcessChat", "Serialize failed for uid:{}", session->GetUniqueId());
        return;
    }

    PacketHead header;
    header.length = htonl(static_cast<uint32_t>(payloadSize));
    header.type = htonl(static_cast<uint32_t>(PacketType::CHAT));
    header.packet_id = htonl(packetId);

    std::vector<char> sendBuf(sizeof(PacketHead) + payloadSize);
    std::memcpy(sendBuf.data(), &header, sizeof(PacketHead));
    std::memcpy(sendBuf.data() + sizeof(PacketHead), payload.data(), payloadSize);

    if (!session->SendPacket(sendBuf.data(), static_cast<uint16_t>(sendBuf.size())))
    {
        LOG_ERR("ProcessChat", "SendPacket failed uid:{} size:{}", session->GetUniqueId(), sendBuf.size());
        return;
    }

    LOG_INFO("Chat", "Echoed back to uid:{} size:{}", session->GetUniqueId(), sendBuf.size());
}

