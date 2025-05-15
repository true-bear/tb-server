#pragma once
#include "pch.h"
#include "threadManager.h"

class RIOCompletionDispatcher
{
public:
    bool Start(RIO_CQ completionQueue);
    void Stop();

private:
    void Run();

    std::unique_ptr<ThreadManager> mThreadManager;

    std::thread mThread;
    std::atomic<bool> mRunning = false;
    RIO_CQ mCQ = RIO_INVALID_CQ;
};
