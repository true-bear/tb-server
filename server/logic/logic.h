#pragma once
#include "pch.h"
#include "Singleton.h"
#include "logic_dispatch.h" 

using SessionGetFunc = std::function<ClientSession* (int)>;

class LogicManager : public Singleton<LogicManager>
{
    friend class Singleton<LogicManager>;

public:
    LogicManager();
    ~LogicManager();

    bool Init(SessionGetFunc sessionObj);
    void Start();
    void Stop();
    void DisPatchPacket(int sessionId, std::span<const std::byte> data);
    void RunThread();

private:
    LogicDispatch mDispatcher;
    boost::lockfree::queue<PacketEx*> mPacketQueue{ PACKET_QUEUE_SIZE };

    std::unique_ptr<ThreadManager> mLogicWorker;
    SessionGetFunc mGetSessioObject;

    std::atomic<bool> mRunning{ false };
};
