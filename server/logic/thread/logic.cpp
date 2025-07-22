#include "pch.h"
#include "logic.h"
#include "..\logic\logic_chat.h"
#include "..\logic\logic_dispatch.h"

LogicThread::LogicThread() : mPacketQueue(PACKET_QUEUE_SIZE), mRunning(false) 
{
    mLogicWorker = std::make_unique<ThreadManager>();
}

LogicThread::~LogicThread()
{
    PacketEx* pkt = nullptr;
    while (mPacketQueue.pop(pkt))
    {
        delete pkt;
    }
}

bool LogicThread::Init(SessionGetFunc sessionObj)
{
    mGetSessioObject = sessionObj;
    mDispatcher.Register(static_cast<size_t>(PacketType::CHAT), ProcessChat);
    return true;
}


void LogicThread::Start()
{
    mRunning.store(true);
}

void LogicThread::Stop()
{
    mRunning.store(false);
}

void LogicThread::RunThread()
{
    while (mRunning.load()) {
        PacketEx* packet = nullptr;

        if (mPacketQueue.pop(packet))
        {
            auto session = mGetSessioObject(packet->sessionId);
            if (!session)
            {
                LOG_ERR("Logic", "session nullptr");
                delete packet;
                continue;
            }

            if (packet->payload.empty())
            {
                LOG_ERR("Logic", "ºó payload ¼ö½ÅµÊ. sessionId: {}", packet->sessionId);
                delete packet;
                continue;
            }

            mDispatcher.Dispatch(
                static_cast<size_t>(packet->type),
                session,
                packet->payload.data(),
                packet->payload.size(),
                packet->packetId
            );

            delete packet;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    LOG_INFO("Logic", "LogicThread Á¾·á");
}


void LogicThread::DisPatchPacket(int sessionId, uint16_t packetType, uint32_t packetId, const char* payload, uint16_t payloadSize)
{
    PacketEx* pkt = new PacketEx(sessionId, packetType, packetId, payload, payloadSize);

    if (!mPacketQueue.push(pkt))
    {
        delete pkt;
        LOG_ERR("Logic", "packet push failed");
        return;
    }

    LOG_INFO("OnRecv", "Recv packet - uID:{} type:{} size:{}", sessionId, packetType, payloadSize);
}

