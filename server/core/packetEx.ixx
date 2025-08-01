module;

export module util.packet;

import <vector>;
import <span>;

export class PacketEx
{
public:
    PacketEx(int sessionId, std::span<const std::byte> data)
        : mSessionId(sessionId), mData(data.size())
    {
        //todo 최적화 필요
        std::memcpy(mData.data(), data.data(), data.size());
    }

    const std::vector<std::byte>& GetData() const { return mData; }
    int GetSessionId() const { return mSessionId; }

private:
    int mSessionId;
    std::vector<std::byte> mData;
};

