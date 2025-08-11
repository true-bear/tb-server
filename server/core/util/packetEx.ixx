module;
#include <cstddef>
#include <cstdint>
#include <array>

export module util.packet;

import common.define;
import common.message;

import <vector>;
import <span>;
import <cstdint>;
import <vector>;
import <cstring>;

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

std::vector<std::byte> AssemblePacket(const MsgHeader& h, std::span<const std::byte> payload)
{
    const uint32_t frameLen = static_cast<uint32_t>(sizeof(MsgHeader) + payload.size());
    const size_t total = sizeof(uint32_t) + frameLen;
    std::vector<std::byte> out(total);

    std::memcpy(out.data(), &frameLen, sizeof(uint32_t));
    std::memcpy(out.data() + sizeof(uint32_t), &h, sizeof(MsgHeader));

    if (!payload.empty())
        std::memcpy(out.data() + sizeof(uint32_t) + sizeof(MsgHeader), payload.data(), payload.size());
    return out;
}

