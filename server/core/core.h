#pragma once
#include "pch.h"
#include "define.h"
#include "iocp/iocp.h"
#include "iocp/socket.h"
#include "iocp/session.h"
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

    void GetIocpEvents(Iocp::IocpEvents& events, unsigned long timeout);

    virtual void OnAccept(const unsigned int uID, unsigned long long completekey) = 0;
    virtual void OnRecv(const unsigned int uID, unsigned long ioSize) = 0;
    virtual bool OnClose(const unsigned int uID) = 0;
    virtual void OnSend(const unsigned int uID, unsigned long ioSize) = 0;

protected:
    SocketEx mListenSocket;
    SessionPool mSessionPool;
    int mMaxSession{ 0 };
    WorkerPool mWorkers;
    std::atomic<bool> mIsRunThread{ false };
};


//class Core : public Iocp
//{
//
//public:
//    Core();
//    virtual ~Core();
//
//    bool Init(const int maxSession);
//    void Stop();
//
//    void WorkerThread(std::stop_token st);
//
//    bool CreateSessionPool();
//
//    Session* GetSession(const unsigned int uID) const;
//    const SOCKET& GetListenSocket() const { return mListenSocket.GetSocket(); }
//    
//    using SessionPool = std::unordered_map<unsigned int, std::unique_ptr<Session>>;
//    SessionPool& GetSessionPool() {return mSessionPool;}
//    bool IsRunThread() const;
//
//    void GetIocpEvents(Iocp::IocpEvents& events, unsigned long timeout);
//
//    virtual void OnAccept(const unsigned int uID, unsigned long long completekey) = 0;
//    virtual void OnRecv(const unsigned int uID, unsigned long ioSize) = 0;
//    virtual bool OnClose(const unsigned int uID) = 0;
//    virtual void OnSend(const unsigned int uID, unsigned long ioSize) = 0;
//
//protected:
//    SocketEx mListenSocket;
//    SessionPool mSessionPool;
//    int mMaxSession{ 0 };
//    std::unique_ptr<ThreadManager> mThreadManager;
//    std::atomic<bool> mIsRunThread{ false };
//};