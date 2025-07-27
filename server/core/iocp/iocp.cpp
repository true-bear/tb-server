#include "pch.h"
#include "iocp.h"

Iocp::~Iocp()
{
    CloseHandle(mIocp);
}

bool Iocp::CreateNewIocp(unsigned long threadCnt)
{
    mIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, threadCnt);

    if (!mIocp)
        return false;

    return true;
}

bool Iocp::AddDeviceListenSocket(SOCKET listenSock)
{
    auto ret = CreateIoCompletionPort(
        (HANDLE)listenSock,
        mIocp,
        (ULONG_PTR)nullptr,
        0
    );

    if (ret != mIocp)
        return false;

    return true;
}

bool Iocp::AddDeviceRemoteSocket(Session* RemoteSession)
{
    HANDLE ret = CreateIoCompletionPort(
        (HANDLE)RemoteSession->GetRemoteSocket(),
        mIocp,
        (ULONG_PTR)&RemoteSession,
        0
    );

    if (ret != mIocp)
        return false;

    return true;
}

void Iocp::GQCSEx(Iocp::IocpEvents& IoEvent, unsigned long timeOut)
{
    bool ret = GetQueuedCompletionStatusEx(
        mIocp,
        IoEvent.m_IoArray,
        IOCP_EVENT_COUNT,
        (ULONG*)&IoEvent.m_eventCount,
        timeOut,
        FALSE
    );

    if (!ret)
        IoEvent.m_eventCount = 0;
}

bool Iocp::PQCS(unsigned long byte, ULONG_PTR completeKey, OVERLAPPED* overlapped)
{
    return PostQueuedCompletionStatus(
        mIocp,
        byte,
        completeKey,
        overlapped);
}