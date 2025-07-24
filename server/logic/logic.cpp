#include "pch.h"
#include "logic.h"
#include "logic_chat.h"
#include "logic_dispatch.h"

LogicManager::LogicManager() : mPacketQueue(PACKET_QUEUE_SIZE), mRunning(false) 
{
    mLogicWorker = std::make_unique<ThreadManager>();
}

LogicManager::~LogicManager()
{
    PacketEx* pkt = nullptr;
    while (mPacketQueue.pop(pkt))
    {
        delete pkt;
    }
}

bool LogicManager::Init(SessionGetFunc sessionObj)
{
    mGetSessioObject = sessionObj;
    mDispatcher.Register(static_cast<size_t>(PacketType::CHAT), ProcessChat);
    return true;
}


void LogicManager::Start()
{
    mRunning.store(true);
}

void LogicManager::Stop()
{
    mRunning.store(false);
}

void LogicManager::RunThread()
{
    while (mRunning.load()) {
        PacketEx* packet = nullptr;

        if (mPacketQueue.pop(packet))
        {
            auto session = mGetSessioObject(packet->GetSessionId());
            if (!session)
            {
                LOG_ERR("Logic", "session nullptr");
                continue;
            }

            const std::span<const std::byte> data = packet->GetData();
            if (data.size() <= sizeof(uint16_t))
            {
                LOG_ERR("Logic", "invalid packet size");
                continue;
            }

            const std::byte* protoStart = data.data() + sizeof(uint16_t);
            const int protoSize = static_cast<int>(data.size() - sizeof(uint16_t));

            PacketHeader header;
            if (!header.ParseFromArray(protoStart, protoSize))
            {
                LOG_ERR("Logic", "PacketHeader 파싱 실패");
                continue;
            }

            PacketType type = header.type();
            mDispatcher.Dispatch(static_cast<size_t>(type), session, data);
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    LOG_INFO("Logic", "LogicThread 종료");
}

void LogicManager::DisPatchPacket(int sessionId, std::span<const std::byte> data)
{
    PacketEx* pkt = new PacketEx(sessionId, data);
    if (!mPacketQueue.push(pkt))
    {
        delete pkt;
        LOG_ERR("Logic", "packet push failed");
    }

    LOG_INFO("OnRecv", "Recv packet - uID:{} : {}", sessionId, data.size());
}
