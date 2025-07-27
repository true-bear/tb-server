#include "pch.h"
#include "core.h"
#include "iocp/iocp.h"
#include "iocp/socket.h"
#include "../util/config.h"
#include "iocp/session.h"


Core::Core() {}

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

    SYSTEM_INFO info;
    GetSystemInfo(&info);
    const int workerCount = info.dwNumberOfProcessors * 2;

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
        mWorkers.push_back(std::move(worker));
    }

    LOG_INFO("Core", "서버 시작. core:{}, worker thread: {} session pool: {}",
        info.dwNumberOfProcessors, workerCount, mMaxSession);

    return true;
}

bool Core::CreateSessionPool()
{
	int total = mMaxSession;

	for (auto i = 0; i < total; ++i)
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
    mIsRunThread = false;

    for (size_t i = 0; i < mWorkers.size(); ++i)
    {
        PostQueuedCompletionStatus(GetIocp(), 0, 0, nullptr);
    }

    for (auto& w : mWorkers)
    {
        w->Stop();
    }

    mSessionPool.clear();
}

bool Core::IsRunThread() const
{
    return mIsRunThread.load(std::memory_order_acquire);
}

Session* Core::GetSession(const unsigned int uID) const
{
	auto it = mSessionPool.find(uID);
	return (it != mSessionPool.end()) ? it->second.get() : nullptr;
}

void Core::GetIocpEvents(Iocp::IocpEvents& events, unsigned long timeout)
{
    GQCSEx(events, timeout);
}

//Core::Core()
//{
//	mThreadManager = std::make_unique<ThreadManager>();
//}
//
//Core::~Core()
//{
//	mSessionPool.clear();
//}
//
//bool Core::Init(int maxSession)
//{
//	if (maxSession <= 0)
//	{
//		LOG_ERR("", "max count:{}", maxSession);
//		return false;
//	}
//
//	mMaxSession = maxSession;
//
//	if (mThreadManager == nullptr)
//	{
//		LOG_ERR("", "thread manager nullptr");
//		return false;
//	}
//
//	if (!mListenSocket.Init())
//	{
//		LOG_ERR("", "listen socket");
//		return false;
//	}
//
//	if (!mListenSocket.BindAndListen())
//	{
//		LOG_ERR("", "BindAndListen");
//		return false;
//	}
//
//	SYSTEM_INFO info;
//	GetSystemInfo(&info);
//	unsigned long workerCount = info.dwNumberOfProcessors * 2;
//
//	if (!CreateNewIocp(workerCount))
//	{
//		LOG_ERR("", "CreateNewIocp worker:{}", workerCount);
//		return false;
//	}
//
//	SOCKET listenSocket = mListenSocket.GetSocket();
//	if (listenSocket == INVALID_SOCKET)
//	{
//		LOG_ERR("", "listenSocket");
//		return false;
//	}
//
//	if (!AddDeviceListenSocket(listenSocket))
//	{
//		LOG_ERR("", "listenSocket");
//		return false;
//	}
//
//	if (!CreateSessionPool())
//	{
//		LOG_ERR("", "CreateSessionPool");
//		return false;
//	}
//
//	mIsRunThread = true;
//	for (auto i = 0; i < workerCount; ++i)
//	{
//		mThreadManager->Run([this](std::stop_token st) 
//		{
//			this->WorkerThread(st);
//		});
//	}
//
//	LOG_INFO("Core", "서버 시작. core:{}, worker thread: {} session pool: {}",
//		info.dwNumberOfProcessors, workerCount, mMaxSession);
//
//	return true;
//}
//
//void Core::Stop()
//{
//    if (!mThreadManager)
//        return;
//
//    mIsRunThread = false;
//
//    const size_t threadCount = mThreadManager->GetThreadCount();
//
//    for (size_t i = 0; i < threadCount; ++i)
//    {
//        PostQueuedCompletionStatus(GetIocp(), 0, 0, nullptr);
//    }
//
//    mThreadManager->Stop();
//	mSessionPool.clear();
//}
//
//
//void Core::WorkerThread(std::stop_token st)
//{
//	while (mIsRunThread && !st.stop_requested())
//	{
//		Iocp::IocpEvents events;
//		GQCSEx(events, 5);
//
//		for (int i = 0; i < events.m_eventCount; ++i)
//		{
//			auto& getIoEvent = events.m_IoArray[i];
//
//			if (getIoEvent.lpOverlapped == nullptr)
//			{
//				LOG_INFO("Core", "thread shutdown");
//				return;
//			}
//
//			unsigned long ioSize = getIoEvent.dwNumberOfBytesTransferred;
//			OverlappedIoEx* over = reinterpret_cast<OverlappedIoEx*>(getIoEvent.lpOverlapped);
//
//			if (!over)
//			{
//				LOG_ERR("Core", "Overlapped nullptr");
//				continue;
//			}
//
//			int sessionID = over->mUID;
//			IO_TYPE ioType = over->mIOType;
//
//			auto session = GetSession(sessionID);
//			if (!session)
//			{
//				LOG_ERR("Core", "session nullptr id:{}", sessionID);
//				continue;
//			}
//
//			if (0 >= ioSize && IO_TYPE::ACCEPT != ioType)
//			{
//				OnClose(sessionID);
//				continue;
//			}
//
//			switch (ioType)
//			{
//			case IO_TYPE::ACCEPT:
//				OnAccept(sessionID, getIoEvent.lpCompletionKey);
//				break;
//			case IO_TYPE::RECV:
//				OnRecv(sessionID, ioSize);
//				break;
//			case IO_TYPE::SEND:
//				OnSend(sessionID, ioSize);
//				break;
//			default:
//				break;
//			}
//		}
//	}
//}
//
//
//bool Core::CreateSessionPool()
//{
//	int total = mMaxSession;
//
//	for (auto i = 0; i < total; ++i)
//	{
//		auto session = std::make_unique<Session>();
//		if (!session)
//			return false;
//
//		session->SetUniqueId(i);
//
//		const auto& listenSocket = mListenSocket.GetSocket();
//		if (!session->AcceptReady(listenSocket, i))
//			return false;
//
//		mSessionPool.emplace(i, std::move(session));
//	}
//	return true;
//}
//
//Session* Core::GetSession(const unsigned int uID) const
//{
//	auto it = mSessionPool.find(uID);
//	return (it != mSessionPool.end()) ? it->second.get() : nullptr;
//}
//
//bool Core::IsRunThread() const
//{
//	return mIsRunThread.load(std::memory_order_acquire);
//}
//
//void Core::GetIocpEvents(Iocp::IocpEvents& events, unsigned long timeout)
//{
//	GQCSEx(events, timeout);
//}