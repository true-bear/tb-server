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

#pragma pack(push, 1)
export struct GameHeader 
{
    uint16_t type;
    uint32_t size;
};
#pragma pack(pop)

export enum class PacketType : uint16_t { CHAT_C2S = 1, CHAT_S2C = 2 };

//export inline std::vector<std::byte> Serialize(PacketType type, const google::protobuf::Message& msg) 
//{
//    std::string body; 
//    msg.SerializeToString(&body);
//
//    GameHeader h{ static_cast<uint16_t>(type), static_cast<uint32_t>(body.size()) };
//    std::vector<std::byte> buf(sizeof(GameHeader) + body.size());
//    std::memcpy(buf.data(), &h, sizeof(GameHeader));
//    std::memcpy(buf.data() + sizeof(GameHeader), body.data(), body.size());
//    return buf;
//}

export inline bool ParseHeader(std::span<const std::byte> frame, GameHeader& out, std::span<const std::byte>& body) 
{
    if (frame.size() < sizeof(GameHeader)) return false;
    std::memcpy(&out, frame.data(), sizeof(GameHeader));
    if (frame.size() < sizeof(GameHeader) + out.size) return false;
    body = frame.subspan(sizeof(GameHeader), out.size);
    return true;
}

