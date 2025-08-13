
export module common.define;

import <cstdint>;

export enum class IO_TYPE
{
    NONE,
    RECV,
    SEND,
    ACCEPT,
	CONNECT,
};

export enum class ServerRole : uint8_t
{
    Client = 0,
    Server = 1
};

