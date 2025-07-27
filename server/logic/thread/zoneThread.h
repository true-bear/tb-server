#pragma once
#include "pch.h"
//#include "../logic_dispatch.h" 
//
//using SessionGetFunc = std::function<ClientSession* (int)>;
//
//class ZoneThread : public Singleton<ZoneThread>
//{
//    friend class Singleton<ZoneThread>;
//
//public:
//    ZoneThread();
//    ~ZoneThread();
//
//    bool Init(SessionGetFunc sessionObj);
//    void Start();
//    void Stop();
//    void DisPatchPacket(int sessionId, std::span<const std::byte> data);
//    void RunThread(std::stop_token st);
//
//private:
//    LogicDispatch mDispatcher;
//    boost::lockfree::queue<PacketEx*> mPacketQueue{ PACKET_QUEUE_SIZE };
//
//    std::unique_ptr<ThreadManager> mLogicWorker;
//    SessionGetFunc mGetSessioObject;
//
//    std::atomic<bool> mRunning{ false };
//};
