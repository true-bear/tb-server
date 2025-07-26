#include "pch.h"
#include "server.h"
#include "logic.h"
LogicServer::LogicServer()
{
    mThread = std::make_unique<ThreadManager>();
}

LogicServer::~LogicServer()
{
    Stop();
    mRecvFuncMap.clear();
    mActiveSessionMap.clear();
}

bool LogicServer::Init(int maxSession)
{
    mMaxSession = maxSession;
    LOG_INFO("LogicServer::Init", "mMaxSession ÃÊ±âÈ­µÊ: {}", mMaxSession);

    if (!Core::Init(maxSession))
    {
        LOG_ERR("Core Init", "** failed **");
        return false;
    }

    
    if (!LogicManager::Get().Init(
        [this](int sessionId) { return this->GetSession(sessionId); }))
    {
        LOG_ERR("logic Init", "** failed **");
        return false;
    }

    return true;
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

        uint16_t packetSize = 0;
        std::span<std::byte> headerView(reinterpret_cast<std::byte*>(&packetSize), sizeof(packetSize));
        if (!recvBuffer->Read(headerView, sizeof(packetSize)))
            return;

        packetSize = ntohs(packetSize);

        if (packetSize == 0)
            return;

        if (storedSize < packetSize)
            continue;
        else 
            recvBuffer->MoveReadPos(static_cast<size_t>(sizeof(packetSize)));

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


void LogicServer::Run()
{
    LogicManager::Get().Start();
    mThread->Run([this](std::stop_token st)
        {
            LogicManager::Get().RunThread(st);
    });
}

void LogicServer::Stop()
{
    LogicManager::Get().Stop();

    Core::Stop();
}
bool LogicServer::OnClose(unsigned int uID)
{
    ClientSession* session{ nullptr };
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

void LogicServer::OnAccept(unsigned int uID, unsigned long long completekey)
{
    auto session = GetSession(uID);
    if (!session || completekey != 0)
        return;

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

    
    std::lock_guard<std::mutex> lock(mActiveSessionLock);

    if ((int)mActiveSessionMap.size() < mMaxSession)
    {
        mActiveSessionMap[uID] = session;
        session->RecvReady();
    }
    
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

void LogicServer::BindSession(ClientSession* session)
{
    std::lock_guard<std::mutex> lock(mActiveSessionLock);
    mActiveSessionMap.emplace(session->GetUniqueId(), session);
    session->RecvReady();
}