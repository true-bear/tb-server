module;
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#include <windows.h>
#include "../define.h"

module iocp.socket;
import util.conf;

bool SocketEx::Init()
{
    WSADATA wsaData;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0)
        return false;

    mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
    return mSocket != INVALID_SOCKET;
}

bool SocketEx::Close()
{
    return closesocket(mSocket) != SOCKET_ERROR;
}

bool SocketEx::BindAndListen()
{
    int port = Config::Load(CATEGORY_NET, PORT);

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

bool SocketEx::SetOption(int level, int optname, const void* optval, int optlen)
{
    return setsockopt(mSocket, level, optname, static_cast<const char*>(optval), optlen) == 0;
}

void SocketEx::Detach()
{
    mSocket = INVALID_SOCKET;
}
