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


