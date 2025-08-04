#pragma once
#include "pch.h"
#include "define.h"

import iface.handler.io;
import iface.handler.event;

import iocp.socket;
import iocp.session;
import iocp;

#include "thread/Worker.h"

using SessionPool = std::unordered_map<unsigned int, std::unique_ptr<Session>>;
using WorkerPool = std::vector<std::unique_ptr<Worker>>;

class Core : public Iocp
{
public:
    Core();
    virtual ~Core();

    bool Init(const int maxSession);
    void Stop();

    bool CreateSessionPool();

    Session* GetSession(const unsigned int uID) const;
    const SOCKET& GetListenSocket() const { return mListenSocket.GetSocket(); }

    bool IsRunThread() const;

    void GetIocpEvents(IocpEvents& events, unsigned long timeout);
    
    void SetIoContext(IIoHandler* context);
	void SetEventContext(IEventHandler* context);

protected:
    SocketEx mListenSocket;
    SessionPool mSessionPool;
    int mMaxSession{ 0 };
    WorkerPool mWorkers;
    IIoHandler* mIoHandler{ nullptr };
	IEventHandler* mEventHandler{ nullptr };
    std::atomic<bool> mIsRunThread{ false };
};
