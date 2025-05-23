#include "pch.h"
#include "waiting.h"
#include "..\server.h"

WaitingThread::WaitingThread()
{
}

WaitingThread::~WaitingThread()
{

}

void WaitingThread::Enqueue(IocpSession* session)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mQueue.push(session);
}

IocpSession* WaitingThread::TryDequeue()
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (mQueue.empty())
        return nullptr;

    auto session = mQueue.front();
    mQueue.pop();
    return session;
}

size_t WaitingThread::Size() const
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mQueue.size();
}

void WaitingThread::Start()
{
    mRunning.store(true);
}

void WaitingThread::Stop()
{
    mRunning.store(false);
}

void WaitingThread::RunThread()
{
    while (mRunning.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        auto session = TryDequeue();
        if (!session)
            continue;

        if (LogicServer::Get().HasFreeSlot())
        {
            LogicServer::Get().BindSession(session);
        }
        else
        {
            Enqueue(session);
            continue;
        }
    }
}