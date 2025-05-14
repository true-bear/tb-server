#pragma once
#include "pch.h"

#include "waiting.h"
#include "iocp\ClientSession.h"
#include "server.h"

WaitingManager::WaitingManager()
{
    mWorker = std::make_unique<ThreadManager>();
}

WaitingManager::~WaitingManager()
{
}

void WaitingManager::Enqueue(ClientSession* session)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mQueue.push(session);
}

ClientSession* WaitingManager::TryDequeue()
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (mQueue.empty())
        return nullptr;

    auto session = mQueue.front();
    mQueue.pop();
    return session;
}

size_t WaitingManager::Size() const
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mQueue.size();
}

void WaitingManager::Start()
{
    mRunning.store(true);
}

void WaitingManager::Stop()
{
    mRunning.store(false);
}

void WaitingManager::RunThread()
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