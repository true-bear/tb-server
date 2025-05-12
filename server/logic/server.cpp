#include "pch.h"
#include "server.h"
#include "waiting.h"
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

bool LogicServer::Init(int maxSession,int maxWaiting,  int backLog)
{
    mMaxSession = maxSession;
    LOG_INFO("LogicServer::Init", "mMaxSession 초기화됨: {}", mMaxSession);

    if (!Core::Init(maxSession, maxWaiting, backLog))
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
        LOG_ERR("OnRecv", "session nulltpr id:{}", uID);
        return;
    }

    auto recvBuffer = session->GetRecvBuffer();
    if (!recvBuffer)
    {
        LOG_ERR("OnRecv", "recv buffer nulltpr id:{}", uID);
        return;
    }

    session->RecvPacket(ioSize);

    while (true)
    {
        const size_t storedSize = recvBuffer->GetStoredSize();
        if (storedSize < sizeof(uint16_t))
            break;

        uint16_t packetSize = 0;
        if (!recvBuffer->Read(reinterpret_cast<char*>(&packetSize), sizeof(uint16_t)))
            return;

        packetSize = ntohs(packetSize);
        if (packetSize == 0)
            return;

        if (storedSize < packetSize)
        {
            recvBuffer->MoveReadPos(-static_cast<int>(sizeof(uint16_t)));
            break;
        }

        char* packetData = recvBuffer->GetReadPtr();
        LogicManager::Get().DisPatchPacket(session->GetUniqueId(), packetData, packetSize);
       
        recvBuffer->MoveReadPos(packetSize);
        //LOG_INFO("OnRecv", "Recv packet - uID:{} : {}", session->GetUniqueId(), packetSize);
    }

    if (!session->RecvReady())
    {
        LOG_WARN("OnRecv", "recv ready failed id:{}", uID);
        OnClose(uID);
        return;
    }

}

void LogicServer::Run()
{
    WaitingManager::Get().Start();

    mThread->Run([]()
    {
            WaitingManager::Get().RunThread();
    });

    LogicManager::Get().Start();
    mThread->Run([]()
    {
            LogicManager::Get().RunThread();
    });
}

void LogicServer::Stop()
{
    WaitingManager::Get().Stop();
    LogicManager::Get().Stop();

    mThread->Join();
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
    if (!session)
        return;

    if (completekey != 0)
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

    {
        std::lock_guard<std::mutex> lock(mActiveSessionLock);
        if ((int)mActiveSessionMap.size() < mMaxSession)
        {
            mActiveSessionMap[uID] = session;
            session->RecvReady();
        }
        else
        {
            WaitingPacket packet;
            auto* header = new PacketHeader();
            header->set_type(PacketType::WAITING);
            header->set_length(packet.ByteSizeLong());
            
            packet.set_allocated_header(header);
            packet.set_message("waiting...");
            packet.set_waiting_number(WaitingManager::Get().Size());

            int size = packet.ByteSizeLong();
            std::vector<char> buf(size);
            if (!packet.SerializeToArray(buf.data(), size))
            {
                LOG_ERR("WaitingPacket", "Serialize 실패");
                return;
            }

            session->SendPacket(buf.data(), size);

            WaitingManager::Get().Enqueue(session);
            LOG_INFO("Waiting Enqueue", "Session {} pushed to waiting queue", uID);
        }
    }
}



void LogicServer::OnSend(unsigned int uID, unsigned long ioSize)
{
    auto session = GetSession(uID);
    if (!session)
        return;

    BufferEx* sendBuffer = session->GetSendBuffer();
    if (sendBuffer)
    {
        sendBuffer->Read(ioSize);
        LOG_INFO("OnSend", "uid:{} byte:{}", uID, ioSize);
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

    LOG_INFO("Waiting Release", "uid:{} ", session->GetUniqueId() );
}