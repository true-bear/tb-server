module;
#pragma once
#pragma comment(lib, "ws2_32")
#pragma comment(lib,"mswsock.lib")

export module common.define;

import <string_view>;

export namespace ConfigKeys {
    inline constexpr std::wstring_view CATEGORY_NET = L"NETWORK";
    inline constexpr std::wstring_view PORT = L"port";
    inline constexpr std::wstring_view BACKLOG = L"backlog";
    inline constexpr std::wstring_view MAX_WORK_THEAD_CNT = L"workerCount";
    inline constexpr std::wstring_view MAX_SESSION_COUNT = L"maxSessionCount";
    inline constexpr std::wstring_view MAX_IOCP_EVENT_CNT = L"maxIocpEventCount";
}

export namespace NetDefaults {
    inline constexpr int RECV_BUFFER_MAX_SIZE = 4096;
    inline constexpr int SEND_BUFFER_MAX_SIZE = 4096;
    inline constexpr int IOCP_EVENT_COUNT = 100;
    inline constexpr int PACKET_QUEUE_SIZE = 4096;
}
