export module session;

#include <vector>
#include <thread>
#include <iostream>
#include <atomic>
#include <memory>
#include <cstdint>
#include <iomanip>  
#include <algorithm>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <mswsock.h>

#include <string_view>
#include <format>
#include <filesystem>
#include <typeindex>
#include <queue>
#include <mutex>
#include <array>
#include <span>

#include <functional>
#include <stop_token>
#include <chrono>

#include <atlstr.h>

import util.roundbuffer;

export class Session
{
public:
    enum class IO_TYPE
    {
        NONE,
        RECV,
        SEND,
        ACCEPT
    };

    struct OverlappedIoEx : public OVERLAPPED
    {
        WSABUF mWsaBuf;
        int mUID;
        IO_TYPE mIOType;

        OverlappedIoEx(IO_TYPE type = IO_TYPE::NONE) :OVERLAPPED{}, mWsaBuf{}, mUID(-1), mIOType(type) {}
    };

public:
    Session();
    ~Session();

    void			Init();
    void			DisconnectFinish();

    bool			AcceptReady(const SOCKET& listenSock, const int uID);
    bool			AcceptFinish(const SOCKET& listenSocket);

    bool			RecvPacket(unsigned long ioSize);
    bool			RecvReady();

    bool			SendPacket(std::span<const std::byte> data);

    bool			SendReady();

    void			SetUniqueId(int id) { mUID = id; }

    unsigned int	GetUniqueId() const { return mUID; }
    const SOCKET& GetRemoteSocket() const { return mRemoteSock.GetSocket(); }
    RoundBuffer* GetRecvBuffer() const { return mRecvBuffer.get(); }
    RoundBuffer* GetSendBuffer() const { return mSendBuffer.get(); }
    char* GetRecvOverlappedBuffer() const;

    bool			InitRemoteSocket() { mRemoteSock.Init(); }
    bool			IsConnected() const;
private:
    SocketEx					mRemoteSock;
    OverlappedIoEx				mRecvOverEx;
    OverlappedIoEx				mSendOverEx;
    OverlappedIoEx              mAcceptOverEx;

    unsigned int				mUID = -1;
    char						mAcceptBuf[64]{};

private:
    std::unique_ptr<RoundBuffer> mRecvBuffer{ std::make_unique<RoundBuffer>(4096) }; //todo 바꿔야함 일단 잠시 하드코딩
    std::unique_ptr<RoundBuffer> mSendBuffer{ std::make_unique<RoundBuffer>(4096) };
};


