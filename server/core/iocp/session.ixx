module;
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mswsock.h>

export module iocp.session;

import <cstdint>;
import <memory>;
import <span>;
import <vector>;
import <functional>;

import iocp.socket;
import util.roundbuffer;

import common.define;
import util.conf;

import <mutex>;

export struct OverlappedIoEx : public OVERLAPPED
{
    WSABUF mWsaBuf;
    int mUID;
    IO_TYPE mIOType;

    OverlappedIoEx(IO_TYPE type = IO_TYPE::NONE) :OVERLAPPED{}, mWsaBuf{}, mUID(-1), mIOType(type) {}


};

export class Session
{
public:
    [[nodiscard]] Session();
    virtual ~Session();

    void			Reset();
    void			DisconnectFinish();

    bool			AcceptReady(const SOCKET& listenSock, const int uID);
    bool			AcceptFinish(const SOCKET& listenSocket);

    bool			RecvPacket(unsigned long ioSize);
    bool			RecvReady();

    bool			SendPacket(std::span<const std::byte> data);
    bool			SendReady();
    void            SendFinish(size_t bytes);
    bool            PostSendLocked();

    void			SetUniqueId(int id) { mUID = id; }
    unsigned int	GetUniqueId() const { return mUID; }

    SocketEx&       GetRemoteSock()       noexcept { return mRemoteSock; }
    const SOCKET&   GetRemoteSocket() noexcept { return mRemoteSock.GetSocket(); }
    void            AttachSocket(SocketEx&& s) noexcept { mRemoteSock = std::move(s); }

    RoundBuffer*    GetRecvBuffer() const { return mRecvBuffer.get(); }
    RoundBuffer*    GetSendBuffer() const { return mSendBuffer.get(); }
    char*           GetRecvOverlappedBuffer() const;

    bool			InitRemoteSocket() { mRemoteSock.Init(); }
    bool			IsConnected() const;

    void            SetRole(ServerRole r) noexcept { mRole = r; }
    ServerRole      GetRole() const noexcept { return mRole; }

    OverlappedIoEx* GetRecvOverEx()    noexcept { return &mRecvOverEx; }
    OverlappedIoEx* GetSendOverEx()    noexcept { return &mSendOverEx; }
    OverlappedIoEx* GetAcceptOverEx()  noexcept { return &mAcceptOverEx; }
    OverlappedIoEx* GetConnectOverEx() noexcept { return &mConnectOverEx; }

    WSAOVERLAPPED*  GetConnectOv()     noexcept { return reinterpret_cast<WSAOVERLAPPED*>(&mConnectOverEx); }
    void            PrepareConnectOv() noexcept;
    bool            SetFinishConnectContext() { return mRemoteSock.FinishConnect(); }
private:
    SocketEx					mRemoteSock;

    OverlappedIoEx				mRecvOverEx;
    OverlappedIoEx				mSendOverEx;
    OverlappedIoEx              mAcceptOverEx;
    OverlappedIoEx              mConnectOverEx;

    unsigned int				mUID = -1;
    char						mAcceptBuf[64]{};

    ServerRole                  mRole{ ServerRole::Client };

	std::mutex                  mSendLock;
    std::atomic<bool>           mSendPending{ false };
private:
    std::unique_ptr<RoundBuffer> mRecvBuffer{ std::make_unique<RoundBuffer>(NetDefaults::RECV_BUFFER_MAX_SIZE) };
    std::unique_ptr<RoundBuffer> mSendBuffer{ std::make_unique<RoundBuffer>(NetDefaults::SEND_BUFFER_MAX_SIZE) };
};


