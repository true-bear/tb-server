#pragma once
#include "pch.h"
#include "core.h"
#include "iocp.h"
#include "socket.h"
#include "threadManager.h"
#include "config.h"
#include "clientSession.h"
#include "logger.h"

Core::Core()
{
	mThreadManager = std::make_unique<ThreadManager>();
}

Core::~Core()
{
	mSessionPool.clear();
}

bool Core::Init(int maxSession, int maxWaiting, int backLog)
{
	if (maxSession <= 0)
	{
		LOG_ERR("", "max count:{}", maxSession);
		return false;
	}

	mMaxSession = maxSession;
	mMaxWaiting = maxWaiting;

	if (mThreadManager == nullptr)
	{
		LOG_ERR("", "thread manager nullptr");
		return false;
	}

	if (!mListenSocket.Init())
	{
		LOG_ERR("", "listen socket");
		return false;
	}

	if (!mListenSocket.BindAndListen(backLog))
	{
		LOG_ERR("", "BindAndListen");
		return false;
	}

	SYSTEM_INFO info;
	GetSystemInfo(&info);
	unsigned long workerCount = info.dwNumberOfProcessors * 2;

	if (!CreateNewIocp(workerCount))
	{
		LOG_ERR("", "CreateNewIocp worker:{}", workerCount);
		return false;
	}

	SOCKET listenSocket = mListenSocket.GetSocket();
	if (listenSocket == INVALID_SOCKET)
	{
		LOG_ERR("", "listenSocket");
		return false;
	}

	if (!AddDeviceListenSocket(listenSocket))
	{
		LOG_ERR("", "listenSocket");
		return false;
	}

	if (!CreateSessionPool())
	{
		LOG_ERR("", "CreateSessionPool");
		return false;
	}

	mIsRunThread = true;
	for (auto i = 0; i < workerCount; ++i)
	{
		mThreadManager->Run([this]() { this->WorkerThread(); });
	}

	LOG_INFO("Core", "서버 시작. core:{}, worker thread: {} session pool: {}",
		info.dwNumberOfProcessors, workerCount, mMaxSession);

	return true;
}

void Core::Stop()
{
    if (!mThreadManager)
        return;

    mIsRunThread = false;

    const size_t threadCount = mThreadManager->GetThreadCount();

    for (size_t i = 0; i < threadCount; ++i)
    {
        PostQueuedCompletionStatus(GetIocp(), 0, 0, nullptr);
    }

    mThreadManager->Stop();
    mThreadManager->Join();

	mSessionPool.clear();
}


void Core::WorkerThread()
{
	while (mIsRunThread)
	{
		Iocp::IocpEvents events;
		GQCSEx(events, 10);

		for (int i = 0; i < events.m_eventCount; ++i)
		{
			auto& getIoEvent = events.m_IoArray[i];

			if (getIoEvent.lpOverlapped == nullptr)
			{
				LOG_INFO("Core", "thread shutdown");
				return;
			}

			unsigned long ioSize = getIoEvent.dwNumberOfBytesTransferred;
			OverlappedIoEx* over = reinterpret_cast<OverlappedIoEx*>(getIoEvent.lpOverlapped);

			if (!over)
			{
				LOG_ERR("Core", "Overlapped nullptr");
				continue;
			}

			int sessionID = over->mUID;
			IO_TYPE ioType = over->mIOType;

			auto session = GetSession(sessionID);
			if (!session)
			{
				LOG_ERR("Core", "session nullptr id:{}", sessionID);
				continue;
			}

			if (0 >= ioSize && IO_TYPE::ACCEPT != ioType)
			{
				LOG_ERR("WorkerThread", "ioSize <= 0. uid={}, ioType={}", sessionID, static_cast<int>(ioType));

				OnClose(sessionID);
				LOG_INFO("Session", "disconnect id:{}", sessionID);
				continue;
			}

			switch (ioType)
			{
			case IO_TYPE::ACCEPT:
				OnAccept(sessionID, getIoEvent.lpCompletionKey);
				LOG_INFO("Session", "connect id:{}", sessionID);
				break;
			case IO_TYPE::RECV:
				OnRecv(sessionID, ioSize);
				break;
			case IO_TYPE::SEND:
				OnSend(sessionID, ioSize);
				break;
			default:
				break;
			}
		}
	}
}


bool Core::CreateSessionPool()
{
	int total = mMaxSession + mMaxWaiting;

	for (auto i = 0; i < total; ++i)
	{
		auto session = std::make_unique<ClientSession>();
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

ClientSession* Core::GetSession(unsigned int uID) const 
{
	auto it = mSessionPool.find(uID);
	return (it != mSessionPool.end()) ? it->second.get() : nullptr;
}

bool Core::IsRunThread() const
{
	return mIsRunThread.load(std::memory_order_acquire);
}