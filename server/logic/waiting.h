#pragma once
#include "pch.h"

class IocpSession;

class WaitingManager : public Singleton<WaitingManager>
{
public:
    WaitingManager();
    virtual ~WaitingManager();
    void Enqueue(IocpSession* session);
    IocpSession* TryDequeue();
    size_t Size() const;

    void Start();
    void Stop();
    void RunThread();

private:
    std::unique_ptr<ThreadManager> mWorker;

    std::queue<IocpSession*> mQueue;
    mutable std::mutex mMutex;

    std::atomic<bool> mRunning{ false };
};
