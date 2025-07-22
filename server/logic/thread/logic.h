#pragma once
#include "pch.h"
#include "..\logic\logic_dispatch.h" 

using SessionGetFunc = std::function<IocpSession* (int)>;

class LogicThread : public Singleton<LogicThread>
{

public:
    LogicThread();
    virtual ~LogicThread();

    bool Init(SessionGetFunc sessionObj);
    void Start();
    void Stop();
    void DisPatchPacket(int sessionId, uint16_t packetType, uint32_t packetId, const char* payload, uint16_t payloadSize);
    void RunThread();

private:
    LogicDispatch mDispatcher;
    boost::lockfree::queue<PacketEx*> mPacketQueue{ PACKET_QUEUE_SIZE };

    std::unique_ptr<ThreadManager> mLogicWorker;
    SessionGetFunc mGetSessioObject;

    std::atomic<bool> mRunning{ false };
};
