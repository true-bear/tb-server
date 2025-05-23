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
    void DisPatchPacket(const int sessionId, const char* data, uint16_t packetSize);
    void RunThread();

private:
    LogicDispatch mDispatcher;
    boost::lockfree::queue<PacketEx*> mPacketQueue{ PACKET_QUEUE_SIZE };

    std::unique_ptr<ThreadManager> mLogicWorker;
    SessionGetFunc mGetSessioObject;

    std::atomic<bool> mRunning{ false };
};
