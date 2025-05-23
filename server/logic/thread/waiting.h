#pragma once
#include "pch.h"

class IocpSession;

class WaitingThread : public Singleton<WaitingThread>
{
public:
    WaitingThread();
    virtual ~WaitingThread();
    void Enqueue(IocpSession* session);
    IocpSession* TryDequeue();
    size_t Size() const;

    void Start();
    void Stop();
    void RunThread();

private:
    std::queue<IocpSession*> mQueue;
    mutable std::mutex mMutex;

    std::atomic<bool> mRunning{ false };
};
