module;

#include <WinSock2.h>
#include <cstdint>

export module iocp.socket;

import <string_view>;

export class [[nodiscard]] SocketEx
{
public:
    SocketEx();
    virtual ~SocketEx();

    SocketEx(const SocketEx&) = delete;
    SocketEx& operator=(const SocketEx&) = delete;

    SocketEx(SocketEx&& other) noexcept;
    SocketEx& operator=(SocketEx&& other) noexcept;

    bool Init();
    bool Close() const;
    bool BindAndListen(int port) const;
    void Detach();

    bool SetSocket(const SOCKET& newSocket);
    bool SetOption(int level, int optname, const void* optval, int optlen) const;
    bool GetPeerName(sockaddr_in& addr) const;
    const SOCKET& GetSocket() const;
    bool GetSocketInfo(WSAPROTOCOL_INFO& info) const;

    bool ConnectEx(std::wstring_view ip, uint16_t port, WSAOVERLAPPED* ov);
    bool FinishConnect() const;

private:
    SOCKET mSocket{ INVALID_SOCKET };
};
