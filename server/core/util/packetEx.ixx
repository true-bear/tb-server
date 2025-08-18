module;
#include <cstddef>
#include <cstdint>
#include <array>
#include <winsock2.h>

export module util.packet;

import common.define;
import common.message;

import <vector>;
import <span>;
import <cstdint>;
import <vector>;
import <cstring>;
import <string>;

using Byte = std::byte;

export class PacketEx
{
public:
    [[nodiscard]] PacketEx(int sessionId, std::span<const Byte> data)
        : mSessionId(sessionId), mData(data.begin(), data.end()) 
    {
    }

	virtual ~PacketEx() = default;

    const std::vector<Byte>& GetData() const { return mData; }
    int GetSessionId() const { return mSessionId; }

private:
    int mSessionId;
    std::vector<Byte> mData;
};


export inline bool UnWrapPacket(std::span<const Byte>& frame, std::uint32_t& outClientSid) noexcept
{
    if (frame.size() < sizeof(std::uint32_t)) return false;

    std::uint32_t cid_be{};
    std::memcpy(&cid_be, frame.data(), sizeof(cid_be));
    outClientSid = ntohl(cid_be);

    frame = frame.subspan(sizeof(std::uint32_t));
    return true;
}

export inline std::vector<Byte> WrapPacket(std::uint32_t clientSid, std::span<const Byte> payload)
{
    std::vector<Byte> buf(sizeof(std::uint32_t) + payload.size());
    const std::uint32_t cid_be = htonl(clientSid);

    std::memcpy(buf.data(), &cid_be, sizeof(cid_be));
    if (!payload.empty())
        std::memcpy(buf.data() + sizeof(std::uint32_t), payload.data(), payload.size());

    return buf;
}

export inline std::vector<Byte> WrapPacket(std::uint32_t clientSid, const std::string& payload)
{
    return WrapPacket(clientSid,
      std::span<const Byte>(reinterpret_cast<const Byte*>(payload.data()),
            payload.size()));
}

