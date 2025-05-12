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
            auto session = mGetSessioObject(packet->mUID);
            if (session)
            {
                const char* packetStart = packet->mData.data();
                const char* protoStart = packetStart + sizeof(uint16_t);

                PacketHeader header;
                if (!header.ParseFromArray(protoStart, packet->mSize - sizeof(uint16_t)))
                {
                    LOG_ERR("Logic", "PacketHeader 파싱 실패");
                    continue;
                }

                PacketType type = header.type();
                mDispatcher.Dispatch(static_cast<size_t>(type), session, packet->mData.data(), packet->mSize);

            }
            else
            {
                LOG_ERR("Logic", "session nullptr");
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    LOG_INFO("Logic", "LogicThread 종료");
}

void LogicManager::DisPatchPacket(const int sessionId, const char* data, uint16_t packetSize)
{
    PacketEx* pkt = new PacketEx(sessionId, packetSize, data);
    if (!mPacketQueue.push(pkt))
    {
        delete pkt; 
        LOG_ERR("Logic", "packet push failed");
    }

    LOG_INFO("OnRecv", "Recv packet - uID:{} : {}", sessionId, packetSize);
}
