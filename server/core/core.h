#pragma once
#include "pch.h"
#include "define.h"
#include "iocp.h"
#include "socket.h"
#include "threadManager.h"
#include "clientSession.h"

class Core : public Iocp
{

public:
    Core();
    virtual ~Core();

    bool Init(const int maxSession);
    void Stop();

    void WorkerThread(std::stop_token st);

    bool CreateSessionPool();

    ClientSession* GetSession(const unsigned int uID) const;
    const SOCKET& GetListenSocket() const { return mListenSocket.GetSocket(); }
    
    using SessionPool = std::unordered_map<unsigned int, std::unique_ptr<ClientSession>>;
    SessionPool& GetSessionPool() {return mSessionPool;}
    bool IsRunThread() const;

    virtual void OnAccept(const unsigned int uID, unsigned long long completekey) = 0;
    virtual void OnRecv(const unsigned int uID, unsigned long ioSize) = 0;
    virtual bool OnClose(const unsigned int uID) = 0;
    virtual void OnSend(const unsigned int uID, unsigned long ioSize) = 0;

protected:
    SocketEx mListenSocket;
    SessionPool mSessionPool;
    int mMaxSession{ 0 };
    std::unique_ptr<ThreadManager> mThreadManager;
    std::atomic<bool> mIsRunThread{ false };
};