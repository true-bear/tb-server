module;
#include <windows.h>

#pragma once
#pragma comment(lib, "ws2_32")
#pragma comment(lib,"mswsock.lib")

export module util.conf;
import <string>;

export namespace Config 
{
    inline int ReadInt(const wchar_t* sec, const wchar_t* key, int def = -1) 
    {
        wchar_t buf[64]{};
        GetPrivateProfileStringW(sec, key, std::to_wstring(def).c_str(), buf, 64, L"..\\network.ini");
        return _wtoi(buf);
    }

    inline std::wstring ReadStr(const wchar_t* sec, const wchar_t* key, const wchar_t* def = L"") 
    {
        wchar_t buf[256]{};
        GetPrivateProfileStringW(sec, key, def, buf, 256, L"..\\network.ini");
        return buf;
    }
}

export namespace ConfigKeys
{
    inline constexpr std::wstring_view CATEGORY_NET = L"NETWORK";
    inline constexpr std::wstring_view PORT = L"port";
    inline constexpr std::wstring_view BACKLOG = L"backlog";
    inline constexpr std::wstring_view MAX_WORK_THEAD_CNT = L"workerCount";
    inline constexpr std::wstring_view MAX_SESSION_COUNT = L"maxSessionCount";
    inline constexpr std::wstring_view MAX_IOCP_EVENT_CNT = L"maxIocpEventCount";
}

export namespace NetDefaults
{
    inline constexpr int RECV_BUFFER_MAX_SIZE = 4096;
    inline constexpr int SEND_BUFFER_MAX_SIZE = 4096;
    inline constexpr int IOCP_EVENT_COUNT = 100;
    inline constexpr int PACKET_QUEUE_SIZE = 4096;
    inline constexpr int MAX_PACKET_SIZE = 4096;
}
