module;
#include "common/define.h"

export module core.engine;

import iface.handler.io;
import iface.handler.event;

import iocp.socket;
import iocp.session;
import iocp;

import thread.worker;

import <unordered_map>;
import <vector>;
import <memory>;


export class Core : public Iocp
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
    SocketEx            mListenSocket;

    using SessionPool = std::unordered_map<unsigned int, std::unique_ptr<Session>>;
    SessionPool         mSessionPool;

    using WorkerPool = std::vector<std::unique_ptr<Worker>>;
    WorkerPool          mWorkers;

    int                 mMaxSession{ 0 };
    IIoHandler*         mIoHandler{ nullptr };
    IEventHandler*      mEventHandler{ nullptr };
    std::atomic<bool>   mIsRunThread{ false };
};
