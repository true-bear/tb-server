#pragma once
#include "../pch.h"
#include "../util/config.h"
#include "session.h"

class Iocp
{
public:
    struct IocpEvents
    {
        OVERLAPPED_ENTRY m_IoArray[IOCP_EVENT_COUNT];
        int m_eventCount;
    };

    ~Iocp();

    bool CreateNewIocp(unsigned long threadCnt);
    bool AddDeviceListenSocket(SOCKET listenSock);
    bool AddDeviceRemoteSocket(Session* RemoteSession);
    void GQCSEx(Iocp::IocpEvents& IoEvent, unsigned long timeOut);
    bool PQCS(unsigned long byte, ULONG_PTR completeKey, OVERLAPPED* overlapped);

    const HANDLE& GetIocp() const { return mIocp; }

private:
    HANDLE mIocp{ INVALID_HANDLE_VALUE };
};