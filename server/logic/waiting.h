#pragma once
#include "pch.h"
#include "Singleton.h"

class ClientSession;

class WaitingManager : public Singleton<WaitingManager>
{
    friend class Singleton<WaitingManager>;

public:
    void Enqueue(ClientSession* session);
    ClientSession* TryDequeue();
    size_t Size() const;

    void Start();
    void Stop();
    void RunThread();  // ������ ���� �Լ�

private:
    WaitingManager();
    ~WaitingManager();

    void ProcessQueue();

private:
    std::unique_ptr<ThreadManager> mWorker;

    std::queue<ClientSession*> mQueue;
    mutable std::mutex mMutex;

    std::atomic<bool> mRunning{ false };
};
