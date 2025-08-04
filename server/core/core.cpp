#include "pch.h"
#include "core.h"

import iocp;
import iocp.socket;
import util.conf;

import iocp.session;

import <memory>;

Core::Core() = default;

Core::~Core()
{
    Stop();
}

bool Core::Init(int maxSession)
{
    if (maxSession <= 0)
        return false;
    
    mMaxSession = maxSession;

    if (!mListenSocket.Init() || !mListenSocket.BindAndListen())
        return false;

    SYSTEM_INFO sysInfo;
    ::GetSystemInfo(&sysInfo);
    const int workerCount = static_cast<int>(sysInfo.dwNumberOfProcessors) * 2;

    if (!CreateNewIocp(workerCount))
        return false;

    if (!AddDeviceListenSocket(mListenSocket.GetSocket()))
        return false;

    if (!CreateSessionPool())
        return false;

	if (!mIoHandler || !mEventHandler)
		return false;
	
    mIsRunThread = true;

    for (int i = 0; i < workerCount; ++i)
    {
        auto worker = std::make_unique<Worker>(mEventHandler, mIoHandler,"worker", i);
        worker->Start();
        mWorkers.emplace_back(std::move(worker));
    }

	std::cout << std::format("Core::Start: mMaxSession = {}\n", mMaxSession);

    return true;
}

bool Core::CreateSessionPool()
{
    for (int i = 0; i < mMaxSession; ++i)
    {
        auto session = std::make_unique<Session>();
        if (!session)
            return false;

        session->SetUniqueId(i);

        const auto& listenSocket = mListenSocket.GetSocket();
        if (!session->AcceptReady(listenSocket, i))
            return false;

        mSessionPool.emplace(i, std::move(session));
    }

    return true;
}

void Core::Stop()
{
    mIsRunThread.store(false, std::memory_order_release);

    for ([[maybe_unused]] const auto& _ : mWorkers)
    {
        ::PostQueuedCompletionStatus(GetIocp(), 0, 0, nullptr);
    }

    for (auto& worker : mWorkers)
    {
        worker->Stop();
    }

    mSessionPool.clear();
}

bool Core::IsRunThread() const
{
    return mIsRunThread.load(std::memory_order_acquire);
}

Session* Core::GetSession(unsigned int uID) const
{
    if (const auto it = mSessionPool.find(uID); it != mSessionPool.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void Core::GetIocpEvents(IocpEvents& events, unsigned long timeout)
{
    GQCSEx(events, timeout);
}

void Core::SetIoContext(IIoHandler* context)
{ 
    mIoHandler = context;
}

void Core::SetEventContext(IEventHandler* context)
{
	mEventHandler = context;
}
