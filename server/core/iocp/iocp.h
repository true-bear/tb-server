#pragma once
#include "pch.h"
#include "util\config.h"
#include "iocpSession.h"

class Iocp
{
public:
    struct IocpEvents
    {
        OVERLAPPED_ENTRY m_IoArray[IOCP_EVENT_COUNT];
        int m_eventCount;
    };

    Iocp() = default;
    ~Iocp() = default;

public:
    bool CreateNewIocp(std::uint32_t threadCnt);
    bool AddDeviceListenSocket(SOCKET listenSock);
    bool AddDeviceRemoteSocket(IocpSession* remoteSession);
    void GQCSEx(Iocp::IocpEvents& ioEvent, std::uint32_t timeOut);
    bool PQCS(std::uint32_t byte, ULONG_PTR completeKey, OVERLAPPED* overlapped);
   
protected:
    HANDLE GetIocpHandle() const noexcept { return mIocp.get(); }

private:
    struct HandleDeleter {
        void operator()(HANDLE handle) const {
            if (handle && handle != INVALID_HANDLE_VALUE)
                CloseHandle(handle);
        }
    };

    using UniqueHandle = std::unique_ptr<std::remove_pointer_t<HANDLE>, HandleDeleter>;
    UniqueHandle mIocp{ nullptr };
};