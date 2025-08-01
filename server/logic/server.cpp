#include "pch.h"
#include "server.h"
#include "logic/logic.h"
import util.singleton;

LogicServer::LogicServer()
{
}

LogicServer::~LogicServer()
{
    Stop();
    mActiveSessionMap.clear();
}

bool LogicServer::Init(int maxSession)
{
    mMaxSession = maxSession;

	std::cout << std::format("LogicServer::Init: mMaxSession = {}\n", mMaxSession);

    if (!Core::Init(maxSession))
    {
		std::cout << "Core Init failed\n";
        return false;
    }

    if (!mLogicManager.Init([this](int sessionId) {
        return this->GetSession(sessionId);
        }))
    {
		std::cout << "LogicManager Init failed\n";
        return false;
    }

    return true;
}

void LogicServer::Run()
{
    mLogicManager.Start();
}

void LogicServer::Stop()
{
    mLogicManager.Stop();
    Core::Stop();
}

void LogicServer::OnRecv(unsigned int uID, unsigned long ioSize)
{
    auto session = GetSession(uID);
    if (!session)
    {
		std::cout << std::format("OnRecv: session not found for id: {}\n", uID);
        return;
    }

    auto recvBuffer = session->GetRecvBuffer();
    if (!recvBuffer)
    {
		std::cout << "OnRecv: recvBuffer null id:" << uID << std::endl;
        return;
    }

    session->RecvPacket(ioSize);

    while (true)
    {
        const size_t storedSize = recvBuffer->ReadableSize();
        if (storedSize < sizeof(uint16_t))
            break;

        uint16_t packetSize = 0;
        std::span<std::byte> headerView(reinterpret_cast<std::byte*>(&packetSize), sizeof(packetSize));
        if (!recvBuffer->Peek(headerView.data(), sizeof(packetSize)))
            return;

        packetSize = ntohs(packetSize);
        if (packetSize == 0) return;

        if (storedSize < sizeof(uint16_t) + packetSize)
            break;

        recvBuffer->MoveReadPos(sizeof(packetSize));

        std::span<const std::byte> packetData{ recvBuffer->ReadPtr(), packetSize };

        LogicManager::Get().DisPatchPacket(uID, packetData);

        recvBuffer->MoveReadPos(packetSize);
    }


    if (!session->RecvReady())
    {
		std::cout << std::format("OnRecv: RecvReady failed for session {}\n", uID);
        OnClose(uID);
    }
}

void LogicServer::OnAccept(unsigned int uID, unsigned long long completekey)
{
    auto session = GetSession(uID);
    if (!session || completekey != 0)
    {
		std::cout << std::format("OnAccept: invalid session or completeKey: {}\n", uID);
        return;
    }

    if (!AddDeviceRemoteSocket(session))
    {
        OnClose(uID);
        return;
    }

    auto listenSocket = GetListenSocket();

    if (!session->AcceptFinish(listenSocket))
    {
        OnClose(uID);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mActiveSessionLock);

        if ((int)mActiveSessionMap.size() < mMaxSession)
        {
            mActiveSessionMap[uID] = session;
            session->RecvReady();
        }
    }
}

bool LogicServer::OnClose(unsigned int uID)
{
    Session* session{ nullptr };
    bool wasActiveSession{ false };

    {
        std::lock_guard<std::mutex> lock(mActiveSessionLock);
        auto it = mActiveSessionMap.find(uID);
        if (it != mActiveSessionMap.end())
        {
            session = it->second;
            mActiveSessionMap.erase(it);
            wasActiveSession = true;
        }
    }

    if (!session)
    {
        session = GetSession(uID);
        if (!session)
        {
			std::cout << std::format("OnClose: session not found for uID: {}\n", uID);
            return false;
        }
    }

    session->DisconnectFinish();
    session->Init();

    if (!session->AcceptReady(GetListenSocket(), uID))
    {
		std::cout << std::format("OnClose: AcceptReady failed for session {}\n", uID);
        return false;
    }

	std::cout << std::format("OnClose: session {} disconnected\n", uID);
    return true;
}

void LogicServer::OnSend(unsigned int uID, unsigned long ioSize)
{
    auto session = GetSession(uID);
    if (!session)
        return;

    auto* sendBuffer = session->GetSendBuffer();
    if (sendBuffer)
    {
        sendBuffer->MoveReadPos(static_cast<size_t>(ioSize));
    }
}

bool LogicServer::HasFreeSlot()
{
    std::lock_guard<std::mutex> lock(mActiveSessionLock);
    return static_cast<int>(mActiveSessionMap.size()) < mMaxSession;
}

void LogicServer::BindSession(Session* session)
{
    std::lock_guard<std::mutex> lock(mActiveSessionLock);
    mActiveSessionMap.emplace(session->GetUniqueId(), session);
    session->RecvReady();
}
