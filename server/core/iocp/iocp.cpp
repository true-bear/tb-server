#include "pch.h"
#include "iocp.h"

bool Iocp::CreateNewIocp(std::uint32_t threadCnt)
{
    mIocp.reset(CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, threadCnt));
    return mIocp != nullptr;
}

bool Iocp::AddDeviceListenSocket(SOCKET listenSock)
{
    HANDLE result = CreateIoCompletionPort(
        reinterpret_cast<HANDLE>(listenSock),
        mIocp.get(),
        reinterpret_cast<ULONG_PTR>(nullptr),
        0
    );

    return result == mIocp.get();
}

bool Iocp::AddDeviceRemoteSocket(IocpSession* remoteSession)
{
    HANDLE result = CreateIoCompletionPort(
        reinterpret_cast<HANDLE>(remoteSession->GetRemoteSocket()),
        mIocp.get(),
        reinterpret_cast<ULONG_PTR>(remoteSession),
        0
    );

    return result == mIocp.get();
}

void Iocp::GQCSEx(Iocp::IocpEvents& ioEvent, std::uint32_t timeOut)
{
    BOOL result = GetQueuedCompletionStatusEx(
        mIocp.get(),
        ioEvent.m_IoArray,
        IOCP_EVENT_COUNT,
        reinterpret_cast<ULONG*>(&ioEvent.m_eventCount),
        timeOut,
        FALSE
    );

    if (!result)
        ioEvent.m_eventCount = 0;
}

bool Iocp::PQCS(std::uint32_t byte, ULONG_PTR completeKey, OVERLAPPED* overlapped)
{
    return PostQueuedCompletionStatus(
        mIocp.get(),
        byte,
        completeKey,
        overlapped
    );
}