module;
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mswsock.h>
#include "../common/define.h"

export module iocp;

import iocp.session;



export struct IocpEvents
{
    OVERLAPPED_ENTRY m_IoArray[IOCP_EVENT_COUNT];
    int m_eventCount;
};


export class Iocp
{
public:
    ~Iocp();

    bool CreateNewIocp(unsigned long threadCnt);
    bool AddDeviceListenSocket(SOCKET listenSock);
    bool AddDeviceRemoteSocket(Session* RemoteSession);
    void GQCSEx(IocpEvents& IoEvent, unsigned long timeOut);
    bool PQCS(unsigned long byte, ULONG_PTR completeKey, OVERLAPPED* overlapped);

    const HANDLE& GetIocp() const { return mIocp; }

private:
    HANDLE mIocp{ INVALID_HANDLE_VALUE };
};