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

    bool Init(int maxSession, int maxWaiting, int backLog);
    void Stop();

    void WorkerThread();

    bool CreateSessionPool();

    ClientSession* GetSession(unsigned int uID) const;
    const SOCKET& GetListenSocket() const { return mListenSocket.GetSocket(); }
    
    using SessionPool = std::unordered_map<unsigned int, std::unique_ptr<ClientSession>>;
    SessionPool& GetSessionPool() {return mSessionPool;}
    bool IsRunThread() const;

    virtual void OnAccept(unsigned int uID, unsigned long long completekey) = 0;
    virtual void OnRecv(unsigned int uID, unsigned long ioSize) = 0;
    virtual bool OnClose(unsigned int uID) = 0;
    virtual void OnSend(unsigned int uID, unsigned long ioSize) = 0;

protected:
    SocketEx mListenSocket;
    SessionPool mSessionPool;
    int mMaxSession{ 0 };
	int mMaxWaiting{ 0 };
    std::unique_ptr<ThreadManager> mThreadManager;
    std::atomic<bool> mIsRunThread{ false };
};