module;

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mswsock.h>

module iocp;
import iocp.socket;
import common.define;

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

void Iocp::GQCSEx(IocpEvents& IoEvent, unsigned long timeOut)
{
    bool ret = GetQueuedCompletionStatusEx(
        mIocp,
        IoEvent.m_IoArray,
        NetDefaults::IOCP_EVENT_COUNT,
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