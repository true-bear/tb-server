#include "pch.h"
#include "define.h"
#include "iocp\iocp.h"
#include "iocp\iocpSocket.h"
#include "threadManager.h"
#include "iocp\iocpSession.h"

class IocpCore : public Iocp
{

public:
    IocpCore();
    virtual ~IocpCore();

    bool Init(int maxSession, int maxWaiting);
    void Stop();

    void WorkerThread();

    bool CreateSessionPool();

    IocpSession* GetSession(unsigned int uID) const;
    const SOCKET& GetListenSocket() const { return mListenSocket.GetSocket(); }
    
    using SessionPool = std::unordered_map<unsigned int, std::unique_ptr<IocpSession>>;
    SessionPool& GetSessionPool() {return mSessionPool;}
    bool IsRunThread() const;

    virtual void OnAccept(unsigned int uID, unsigned long long completekey) = 0;
    virtual void OnRecv(unsigned int uID, unsigned long ioSize) = 0;
    virtual bool OnClose(unsigned int uID) = 0;
    virtual void OnSend(unsigned int uID, unsigned long ioSize) = 0;

protected:
    IocpSocket mListenSocket;
    SessionPool mSessionPool;
    int mMaxSession{ 0 };
	int mMaxWaiting{ 0 };
    std::unique_ptr<ThreadManager> mThreadManager;
    std::atomic<bool> mIsRunThread{ false };
};