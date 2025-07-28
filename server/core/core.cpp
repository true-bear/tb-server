#include "pch.h"
#include "core.h"
#include "iocp/iocp.h"
#include "iocp/socket.h"
#include "../util/config.h"
#include "iocp/session.h"

Core::Core() = default;

Core::~Core()
{
    Stop();
}

bool Core::Init(int maxSession)
{
    if (maxSession <= 0)
    {
        LOG_ERR("Core", "invalid maxSession {}", maxSession);
        return false;
    }

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

    mIsRunThread = true;

    for (int i = 0; i < workerCount; ++i)
    {
        auto worker = std::make_unique<Worker>(this, "worker", i);
        worker->Start();
        mWorkers.emplace_back(std::move(worker));
    }

    LOG_INFO("Core", "서버 시작. core:{}, worker thread: {} session pool: {}",
        sysInfo.dwNumberOfProcessors, workerCount, mMaxSession);

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

void Core::GetIocpEvents(Iocp::IocpEvents& events, unsigned long timeout)
{
    GQCSEx(events, timeout);
}
