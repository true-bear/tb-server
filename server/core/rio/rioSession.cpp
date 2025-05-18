#include "pch.h"
#include "rioSession.h"
#include "rio.h"

RIOSession::RIOSession()
    : mRecvBuffer(8192), mSendBuffer(8192) {
}

RIOSession::~RIOSession()
{
    if (mSocket != INVALID_SOCKET)
        closesocket(mSocket);
}

bool RIOSession::Init(SOCKET socket, RIO_CQ recvCQ, RIO_CQ sendCQ, size_t bufferSize)
{
    mSocket = socket;

    mRequestQueue = RIOFns::Table.RIOCreateRequestQueue(
        socket,
        1, 1,  // recv entries
        1, 1,  // send entries
        recvCQ,
        sendCQ,
        nullptr
    );

    return mRequestQueue != RIO_INVALID_RQ;
}

bool RIOSession::PostRecv()
{
    RIO_BUF buf;
    buf.BufferId = mRecvBuffer.GetBufferId();
    buf.Offset = (ULONG)mRecvBuffer.GetWriteOffset();
    buf.Length = (ULONG)mRecvBuffer.GetWritableSize();

    return RIOFns::Table.RIOReceive(mRequestQueue, &buf, 1, 0, this);
}

bool RIOSession::PostSend(const char* data, size_t size)
{
    if (size > mSendBuffer.GetWritableSize())
        return false;

    std::memcpy(mSendBuffer.GetWritePointer(), data, size);
    mSendBuffer.CommitWrite(size);

    RIO_BUF buf;
    buf.BufferId = mSendBuffer.GetBufferId();
    buf.Offset = (ULONG)mSendBuffer.GetReadOffset();
    buf.Length = (ULONG)mSendBuffer.GetReadableSize();

    return RIOFns::Table.RIOSend(mRequestQueue, &buf, 1, 0, this);
}

void RIOSession::OnRecvCompleted(size_t transferred)
{
    mRecvBuffer.CommitWrite(transferred);
    
    // TODO: ��Ŷ ó�� ���� ȣ��

    mRecvBuffer.CommitRead(transferred);
    PostRecv();
}

void RIOSession::OnSendCompleted(size_t transferred)
{
    mSendBuffer.CommitRead(transferred);
    // TODO: ���� ��� ���� ���� ó���� �� ����
}
