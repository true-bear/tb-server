#include "../pch.h"
#include "../logic.h"
#include "logic_chat.h"


boost::lockfree::queue<PacketEx*> LogicManager::mPacketQueue{ NetDefaults::PACKET_QUEUE_SIZE };

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
    mDispatcher.Register(static_cast<size_t>(PacketType::CHAT),&ProcessChat);

    mLogicThread = std::make_unique<LogicThread>(
        "LogicThread",
        std::move(getSession),
        mDispatcher, 
		mPacketQueue
    );

    return true;
}

void LogicManager::Start()
{
    if (mLogicThread)
        mLogicThread->Start();
}

void LogicManager::Stop()
{
    if (mLogicThread)
        mLogicThread->Stop();
}

void LogicManager::DisPatchPacket(int sessionId, std::span<const std::byte> data)
{
    PacketEx* pkt = new PacketEx(sessionId, data);
    if (!mPacketQueue.push(pkt))
    {
        delete pkt;
		std::cout << std::format("DisPatchPacket: packet push failed - queue full? sessionId: {}, data size: {}\n", sessionId, data.size());
    }

	std::cout << std::format("DisPatchPacket: Recv packet - uID:{} : {}\n", sessionId, data.size());
}
