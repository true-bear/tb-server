#include "pch.h"
#include "rioSocket.h"

bool RIOSocket::Init(int port)
{
    mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_REGISTERED_IO);
    if (mListenSocket == INVALID_SOCKET)
    {
        std::cerr << "[RIO] WSASocket 실패: " << WSAGetLastError() << std::endl;
        return false;
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (::bind(mListenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        std::cerr << "[RIO] bind 실패: " << WSAGetLastError() << std::endl;
        return false;
    }

    if (listen(mListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "[RIO] listen 실패: " << WSAGetLastError() << std::endl;
        return false;
    }

    return true;
}

SOCKET RIOSocket::Accept()
{
    SOCKET client = accept(mListenSocket, nullptr, nullptr);
    if (client == INVALID_SOCKET)
    {
        std::cerr << "[RIO] accept 실패: " << WSAGetLastError() << std::endl;
    }
    return client;
}
