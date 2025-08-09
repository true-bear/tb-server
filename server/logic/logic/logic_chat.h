#pragma once
class Session;

void ProcessChat(Session* session, const std::byte* data, size_t size)
{
    if (!session)
    {
		std::cout << std::format("ProcessChat: session is nullptr\n");
        return;
    }

    ChatPacket chatPacket;
    if (!chatPacket.ParseFromArray(data, static_cast<int>(size)))
        return;
    

    const int serializedSize = chatPacket.ByteSizeLong();
    std::vector<std::byte> serializedData(serializedSize);
    if (!chatPacket.SerializeToArray(reinterpret_cast<void*>(serializedData.data()), serializedSize))
    {
		std::cout << std::format("ProcessChat: SerializeToArray failed uid:{} size:{}\n", session->GetUniqueId(), serializedSize);
        return;
    }

    const std::span<const std::byte> serializedSpan = serializedData;
    if (!session->SendPacket(serializedSpan))
    {
		std::cout << std::format("ProcessChat: SendPacket failed uid:{} size:{}\n", session->GetUniqueId(), serializedData.size());
		return;
    }

	std::cout << std::format("Chat: uid:{} msg:'{}'\n", session->GetUniqueId(), chatPacket.message());
}
