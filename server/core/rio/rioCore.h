#pragma once
#include "pch.h"
#include "rioSocket.h"
#include "rioSession.h"

class RIOCompletionDispatcher
{
public:
    bool Start(RIO_CQ completionQueue);
    void Stop();

private:
    void Run();

    std::thread mThread;
    std::atomic<bool> mRunning = false;
    RIO_CQ mCQ = RIO_INVALID_CQ;
};

class RIOCore
{
public:
    RIOCore();
    ~RIOCore();

    bool Init(int port, int maxSession);
    void Run();
    void Stop();

private:
    SOCKET mListenSocket = INVALID_SOCKET;
    std::vector<std::unique_ptr<RIOSession>> mSessionPool;

    RIO_CQ mRecvCQ = RIO_INVALID_CQ;
    RIO_CQ mSendCQ = RIO_INVALID_CQ;

    std::unique_ptr<RIOCompletionDispatcher> mDispatcher;
    std::unique_ptr<RIOSocket> mSocket;

    std::atomic<bool> mRunning = false;
};
