#pragma once
#include "pch.h"
#include "rioBuffer.h"

class RIOSession
{
public:
    RIOSession();
    ~RIOSession();

    bool Init(SOCKET socket, RIO_CQ recvCQ, RIO_CQ sendCQ, size_t bufferSize = 8192);

    bool PostRecv();
    bool PostSend(const char* data, size_t size);

    void OnRecvCompleted(size_t transferred);
    void OnSendCompleted(size_t transferred);

    SOCKET GetSocket() const { return mSocket; }
    RIO_RQ GetRequestQueue() const { return mRequestQueue; }

private:
    SOCKET mSocket = INVALID_SOCKET;
    RIO_RQ mRequestQueue = RIO_INVALID_RQ;

    RioBuffer mRecvBuffer;
    RioBuffer mSendBuffer;
};
