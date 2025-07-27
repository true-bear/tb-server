#include "pch.h"
#include "logic.h"
#include "logic_chat.h"
#include "logic_dispatch.h"

boost::lockfree::queue<PacketEx*> LogicManager::mPacketQueue{ PACKET_QUEUE_SIZE };

LogicManager::LogicManager()
{
}

LogicManager::~LogicManager()
{
    Stop();

    PacketEx* pkt = nullptr;
    while (mPacketQueue.pop(pkt))
    {
        delete pkt;
    }
}

bool LogicManager::Init(SessionGetFunc getSession)
{
    mDispatcher.Register(static_cast<size_t>(PacketType::CHAT), ProcessChat);

    mLogicWorker = std::make_unique<LogicWorker>(
        "LogicWorker",
        std::move(getSession),
        mDispatcher, 
		mPacketQueue
    );

    return true;
}

void LogicManager::Start()
{
    if (mLogicWorker)
        mLogicWorker->Start();
}

void LogicManager::Stop()
{
    if (mLogicWorker)
        mLogicWorker->Stop();
}

void LogicManager::DisPatchPacket(int sessionId, std::span<const std::byte> data)
{
    PacketEx* pkt = new PacketEx(sessionId, data);
    if (!mPacketQueue.push(pkt))
    {
        delete pkt;
        LOG_ERR("LogicManager", "packet push failed - queue full?");
    }


    LOG_INFO("DisPatchPacket", "Recv packet - uID:{} : {}", sessionId, data.size());
}
