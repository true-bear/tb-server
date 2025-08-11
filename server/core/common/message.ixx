export module common.message;

import <cstdint>;

export constexpr uint16_t MSG_MAGIC = 0xBEEF;
export constexpr uint8_t  MSG_VERSION = 1;

export enum class MessageType : uint16_t 
{
    C2S_Ping = 100,
    C2S_Chat = 150,
    S2C_Chat = 250,
};

#pragma pack(push,1)
export struct MsgHeader 
{
    uint16_t magic{ MSG_MAGIC };
    uint8_t  version{ MSG_VERSION };
    uint8_t  pad{ 0 };
    uint16_t type{ 0 };
    uint16_t rsv{ 0 };
    uint64_t dstSessionId{ 0 };
    uint32_t rsv2{ 0 };
};
#pragma pack(pop)

static_assert(sizeof(MsgHeader) == 20, "MsgHeader size invalid...!");
