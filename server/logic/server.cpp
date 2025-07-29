#include "pch.h"
#include "server.h"
#include "logic/logic.h"

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
    LOG_INFO("LogicServer::Init", "mMaxSession 초기화됨: {}", mMaxSession);

    if (!Core::Init(maxSession))
    {
        LOG_ERR("Core Init", "** failed **");
        return false;
    }

    if (!mLogicManager.Init([this](int sessionId) {
        return this->GetSession(sessionId);
        }))
    {
        LOG_ERR("LogicManager Init", "** failed **");
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
        LOG_ERR("OnRecv", "session null id:{}", uID);
        return;
    }

    auto recvBuffer = session->GetRecvBuffer();
    if (!recvBuffer)
    {
        LOG_ERR("OnRecv", "recv buffer null id:{}", uID);
        return;
    }

    session->RecvPacket(ioSize);

    while (true)
    {
        const size_t storedSize = recvBuffer->ReadableSize();
        if (storedSize < sizeof(uint16_t))
            break;

        // 1단계: 패킷 헤더만 Peek (MoveReadPos 하지 않음)
        uint16_t packetSize = 0;
        std::span<std::byte> headerView(reinterpret_cast<std::byte*>(&packetSize), sizeof(packetSize));
        if (!recvBuffer->Peek(headerView.data(), sizeof(packetSize)))
            return;

        packetSize = ntohs(packetSize);
        if (packetSize == 0) return;

        if (storedSize < sizeof(uint16_t) + packetSize)
            break;

        // 2단계: 헤더 스킵
        recvBuffer->MoveReadPos(sizeof(packetSize));

        // 3단계: 본문 파싱
        std::span<const std::byte> packetData{ recvBuffer->ReadPtr(), packetSize };

        LogicManager::Get().DisPatchPacket(uID, packetData);

        recvBuffer->MoveReadPos(packetSize);
    }


    if (!session->RecvReady())
    {
        LOG_WARN("OnRecv", "recv ready failed id:{}", uID);
        OnClose(uID);
    }
}

void LogicServer::OnAccept(unsigned int uID, unsigned long long completekey)
{
    auto session = GetSession(uID);
    if (!session || completekey != 0)
    {
        LOG_ERR("OnAccept", "invalid session or completeKey:{}", uID);
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
            LOG_ERR("OnClose", "Session not found: {}", uID);
            return false;
        }
    }

    session->DisconnectFinish();
    session->Init();

    if (!session->AcceptReady(GetListenSocket(), uID))
    {
        LOG_ERR("OnClose", "AcceptReady failed: {}", uID);
        return false;
    }

    LOG_INFO("Session", "disconnect id:{}", uID);
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
