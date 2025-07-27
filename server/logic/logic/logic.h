#pragma once
#include "pch.h"
#include "util/Singleton.h"
#include "logic_dispatch.h" 
#include "../logic/thread/logicWorker.h"

using SessionGetFunc = std::function<Session* (int)>;

class LogicManager : public Singleton<LogicManager>
{
public:
    LogicManager();
    ~LogicManager();

    bool Init(SessionGetFunc getSession);
    void Start();
    void Stop();

    void DisPatchPacket(int sessionId, std::span<const std::byte> data);

private:
    std::unique_ptr<LogicWorker> mLogicWorker;
    LogicDispatch mDispatcher;
    static boost::lockfree::queue<PacketEx*> mPacketQueue;
    SessionGetFunc mGetSessionFunc;
};

