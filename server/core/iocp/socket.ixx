module;

#include <WinSock2.h>
#include <cstdint>

export module iocp.socket;

export class SocketEx
{
public:
	[[nodiscard]] SocketEx() = default;
	virtual  ~SocketEx() { Close(); }

    SocketEx(const SocketEx&) = delete;
    SocketEx& operator=(const SocketEx&) = delete;

    SocketEx(SocketEx&& other) noexcept { mSocket = other.mSocket; other.mSocket = INVALID_SOCKET; }
    SocketEx& operator=(SocketEx&& other) noexcept {
        if (this != &other) {
            Close();
            mSocket = other.mSocket;
            other.mSocket = INVALID_SOCKET;
        }
        return *this;
    }

    bool Init();
    bool Close() const;
    bool BindAndListen(const int port) const;
    void Detach();

    bool SetSocket(const SOCKET& newSocket) { mSocket = newSocket; return true; }
    bool SetOption(int level, int optname, const void* optval, int optlen) const;
    bool GetPeerName(sockaddr_in& addr) const;
    const SOCKET& GetSocket() const { return mSocket; }
    bool GetSocketInfo(WSAPROTOCOL_INFO& info) const;

    bool ConnectEx(const wchar_t* ip, uint16_t port, WSAOVERLAPPED* ov);
    bool FinishConnect() const;

private:
    SOCKET mSocket{ INVALID_SOCKET };
};
