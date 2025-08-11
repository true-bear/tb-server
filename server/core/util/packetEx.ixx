module;
#include <cstddef>
#include <cstdint>
#include <array>

export module util.packet;

import common.define;

import <vector>;
import <span>;

export class PacketEx
{
public:
    [[nodiscard]] PacketEx(int sessionId, std::span<const std::byte> data)
        : mSessionId(sessionId), mData(data.begin(), data.end()) 
    {
    }

	virtual ~PacketEx() = default;

    const std::vector<std::byte>& GetData() const { return mData; }
    int GetSessionId() const { return mSessionId; }

private:
    int mSessionId;
    std::vector<std::byte> mData;
};

export struct FrameView 
{
    int mSessionId;
    int mType;
    std::span<const std::byte> mPayLoad;
};


