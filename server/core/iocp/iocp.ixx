module;
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mswsock.h>

export module iocp;

import iocp.session;
import common.define;
import util.conf;

export struct IocpEvents
{
    OVERLAPPED_ENTRY m_IoArray[NetDefaults::IOCP_EVENT_COUNT];
    int m_eventCount;
};


export class Iocp
{
public:
	Iocp() = default;
    virtual ~Iocp();

    bool CreateNewIocp(unsigned long threadCnt);
    bool AddDeviceListenSocket(SOCKET listenSock) const;
    bool AddDeviceRemoteSocket(Session* RemoteSession) const;
    void GQCSEx(IocpEvents& IoEvent, unsigned long timeOut) const;
    bool PQCS(unsigned long byte, ULONG_PTR completeKey, OVERLAPPED* overlapped) const;

    const HANDLE& GetIocp() const { return mIocp; }

private:
    HANDLE mIocp{ INVALID_HANDLE_VALUE };
};