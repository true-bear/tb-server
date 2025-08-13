module;
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#include <windows.h>

module iocp.socket;
import util.conf;
import common.define;

bool SocketEx::Init()
{
    WSADATA wsaData;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0)
        return false;

    mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
    return mSocket != INVALID_SOCKET;
}

bool SocketEx::Close() const
{
    return closesocket(mSocket) != SOCKET_ERROR;
}

bool SocketEx::BindAndListen(const int port) const
{
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (::bind(mSocket, (sockaddr*)&addr, sizeof(addr)) != 0)
        return false;

    return ::listen(mSocket, SOMAXCONN) == 0;
}

bool SocketEx::GetPeerName(sockaddr_in& addr) const
{
    int addrlen = sizeof(addr);
    return getpeername(mSocket, reinterpret_cast<sockaddr*>(&addr), &addrlen) == 0;
}

bool SocketEx::GetSocketInfo(WSAPROTOCOL_INFO& info) const
{
    int len = sizeof(info);
    return getsockopt(mSocket, SOL_SOCKET, SO_PROTOCOL_INFO, reinterpret_cast<char*>(&info), &len) == 0;
}

bool SocketEx::SetOption(int level, int optname, const void* optval, int optlen) const
{
    return setsockopt(mSocket, level, optname, static_cast<const char*>(optval), optlen) == 0;
}

void SocketEx::Detach()
{
    mSocket = INVALID_SOCKET;
}


bool SocketEx::ConnectEx(const wchar_t* ip, uint16_t port, WSAOVERLAPPED* ov)
{
    if (mSocket == INVALID_SOCKET && !Init())
        return false;

    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = 0;
    if (::bind(mSocket, reinterpret_cast<sockaddr*>(&local), sizeof(local)) != 0)
        return false;
    

    sockaddr_in remote{};
    remote.sin_family = AF_INET;
    remote.sin_port = htons(port);
    {
        IN_ADDR in{};
        if (InetPtonW(AF_INET, ip, &in) != 1)
            return false;
        remote.sin_addr = in;
    }

    LPFN_CONNECTEX pConnectEx = nullptr;
    {
        GUID guid = WSAID_CONNECTEX;
        DWORD bytes = 0;
        if (WSAIoctl(mSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
            &guid, sizeof(guid),
            &pConnectEx, sizeof(pConnectEx),
            &bytes, nullptr, nullptr) != 0)
        {
            return false;
        }
    }

    BOOL ok = pConnectEx(mSocket,
        reinterpret_cast<sockaddr*>(&remote), sizeof(remote),
        nullptr, 0, nullptr,
        ov);

    if (!ok) 
    {
        const int e = WSAGetLastError();
        if (e != WSA_IO_PENDING)
            return false;
    }

    return true;
}

bool SocketEx::FinishConnect() const
{
    return ::setsockopt(mSocket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, nullptr, 0) == 0;
}