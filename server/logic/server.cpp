#include "pch.h"
#include "server.h"
#include "thread\waiting.h"
#include "thread\logic.h"
#include "thread\db.h"

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

bool LogicServer::Init(int maxSession,int maxWaiting)
{
    mMaxSession = maxSession;
    LOG_INFO("LogicServer::Init", "mMaxSession 초기화됨: {}", mMaxSession);

    if (!IocpCore::Init(maxSession, maxWaiting))
    {
        LOG_ERR("IocpCore Init", "** failed **");
        return false;
    }

    if (!LogicThread::Get().Init(
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
        LOG_ERR("OnRecv", "session nullptr id:{}", uID);
        return;
    }

    auto recvBuffer = session->GetRecvBuffer();
    if (!recvBuffer)
    {
        LOG_ERR("OnRecv", "recv buffer nullptr id:{}", uID);
        return;
    }

    session->RecvPacket(ioSize);

    while (true)
    {
        constexpr size_t HEADER_SIZE = sizeof(PacketHead);
        if (recvBuffer->GetStoredSize() < HEADER_SIZE)
            break;

        std::array<std::byte, HEADER_SIZE> headerBytes;
        if (!recvBuffer->Peek(headerBytes))
        {
            LOG_ERR("OnRecv", "peek failed. storedSize:{} id:{}", recvBuffer->GetStoredSize(), uID);
            break;
        }

        PacketHead header;
        std::memcpy(&header, headerBytes.data(), HEADER_SIZE);

        header.length = ntohs(header.length);
        header.type = ntohs(header.type);
        header.packet_id = ntohl(header.packet_id);

        if (header.length == 0 || header.length > 0x1000)
        {
            LOG_ERR("OnRecv", "invalid header.length:{} storedSize:{} id:{}", header.length, recvBuffer->GetStoredSize(), uID);
            recvBuffer->CommitRead(HEADER_SIZE);
            continue;
        }

        const size_t totalPacketSize = HEADER_SIZE + header.length;
        if (recvBuffer->GetStoredSize() < totalPacketSize)
            break;

        std::vector<std::byte> fullPacket(totalPacketSize);
        if (!recvBuffer->Read(fullPacket))
        {
            LOG_ERR("OnRecv", "read fail. totalSize:{} id:{}", totalPacketSize, uID);
            return;
        }

        const char* payloadPtr = reinterpret_cast<const char*>(fullPacket.data() + HEADER_SIZE);
        const size_t payloadSize = header.length;

        LogicThread::Get().DisPatchPacket(session->GetUniqueId(), header.type, header.packet_id, payloadPtr, payloadSize);
    }

    if (!session->RecvReady())
    {
        LOG_WARN("OnRecv", "recv ready failed id:{}", uID);
        OnClose(uID);
    }
}


void LogicServer::Run()
{
    WaitingThread::Get().Start();

    mThread->Run([]()
    {
            WaitingThread::Get().RunThread();
    });

    LogicThread::Get().Start();
    mThread->Run([]()
    {
            LogicThread::Get().RunThread();
    });

	DBThread::Get().Start();

	mThread->Run([]()
	{
            DBThread::Get().RunThread();
	});

}

void LogicServer::Stop()
{
    WaitingThread::Get().Stop();
    LogicThread::Get().Stop();
	DBThread::Get().Stop();

    mThread->Join();

    IocpCore::Stop();
}
bool LogicServer::OnClose(unsigned int uID)
{
    IocpSession* session{ nullptr };
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
            packet.set_message("waiting...");
            packet.set_waiting_number(WaitingThread::Get().Size());

            const size_t payloadSize = packet.ByteSizeLong();
            std::vector<char> payload(payloadSize);

            if (!packet.SerializeToArray(payload.data(), static_cast<int>(payloadSize)))
            {
                LOG_ERR("WaitingPacket", "Serialize 실패");
                return;
            }

            PacketHead header;
            header.length = htons(static_cast<uint16_t>(payloadSize));       
            header.type = htons(static_cast<uint16_t>(PacketType::WAITING));
            header.packet_id = htonl(0);

            std::vector<char> finalBuf(sizeof(PacketHead) + payloadSize);
            std::memcpy(finalBuf.data(), &header, sizeof(PacketHead));
            std::memcpy(finalBuf.data() + sizeof(PacketHead), payload.data(), payloadSize);

            session->SendPacket(finalBuf.data(), static_cast<uint32_t>(finalBuf.size()));

            WaitingThread::Get().Enqueue(session);
            LOG_INFO("Waiting Enqueue", "Session {} pushed to waiting queue", uID);
        }
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
        if (!sendBuffer->CommitRead(ioSize))
        {
            LOG_WARN("OnSend", "Consume overflow, id:{}, size:{}", uID, ioSize);
        }
    }
}


bool LogicServer::HasFreeSlot()
{
    std::lock_guard<std::mutex> lock(mActiveSessionLock);
    return static_cast<int>(mActiveSessionMap.size()) < mMaxSession;
}

void LogicServer::BindSession(IocpSession* session)
{
    std::lock_guard<std::mutex> lock(mActiveSessionLock);
    mActiveSessionMap.emplace(session->GetUniqueId(), session);
    session->RecvReady();

    LOG_INFO("Waiting Release", "uid:{} ", session->GetUniqueId() );
}