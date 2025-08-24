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

namespace packet_util 
{
    export inline bool UnWrapPacket(std::span<const Byte>& frame, std::uint32_t& outClientSid) noexcept
    {
        if (frame.size() < sizeof(std::uint32_t))
            return false;

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

    export inline constexpr std::size_t kTypeSize = sizeof(std::uint16_t);
    export inline constexpr std::size_t kLenSize = sizeof(std::uint32_t);
    export inline constexpr std::size_t kHeaderSize = kTypeSize + kLenSize;

    export inline bool ReadHeader(std::span<const std::byte> frame, std::uint16_t& outType, std::uint32_t& outLen) noexcept
    {
        if (frame.size() < kHeaderSize) 
            return false;

        std::uint16_t  netType{};
        std::uint32_t netLen{};

        std::memcpy(&netType, frame.data(), kTypeSize);
        std::memcpy(&netLen, frame.data() + kTypeSize, kLenSize);

        outType = ntohs(netType);
        outLen = ntohl(netLen);
        return true;
    }

}

