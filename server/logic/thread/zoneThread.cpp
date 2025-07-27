#include "pch.h"
//#include "logic.h"
//#include "logic_chat.h"
//#include "../logic_dispatch.h"
//
//ZoneThread::ZoneThread() : mPacketQueue(PACKET_QUEUE_SIZE), mRunning(false) 
//{
//    mLogicWorker = std::make_unique<ThreadManager>();
//}
//
//ZoneThread::~ZoneThread()
//{
//    PacketEx* pkt = nullptr;
//    while (mPacketQueue.pop(pkt))
//    {
//        delete pkt;
//    }
//}
//
//bool ZoneThread::Init(SessionGetFunc sessionObj)
//{
//    mGetSessioObject = sessionObj;
//    mDispatcher.Register(static_cast<size_t>(PacketType::CHAT), ProcessChat);
//    return true;
//}
//
//
//void ZoneThread::Start()
//{
//    mRunning.store(true);
//	mLogicWorker->Run([this](std::stop_token st) 
//    {
//	    this->RunThread(st);
//	});
//}
//
//void ZoneThread::Stop()
//{
//    mRunning.store(false);
//    //mLogicWorker->Stop();
//}
//
//void ZoneThread::RunThread(std::stop_token st)
//{
//    while (mRunning && !st.stop_requested())
//    {
//        PacketEx* packet = nullptr;
//
//        if (mPacketQueue.pop(packet))
//        {
//            auto session = mGetSessioObject(packet->GetSessionId());
//            if (!session)
//            {
//                LOG_ERR("Logic", "session nullptr");
//                continue;
//            }
//
//            const std::span<const std::byte> data = packet->GetData();
//            if (data.size() <= sizeof(uint16_t))
//            {
//                LOG_ERR("Logic", "invalid packet size");
//                continue;
//            }
//
//            const std::byte* protoStart = data.data() + sizeof(uint16_t);
//            const int protoSize = static_cast<int>(data.size() - sizeof(uint16_t));
//
//            PacketHeader header;
//            if (!header.ParseFromArray(protoStart, protoSize))
//            {
//                LOG_ERR("Logic", "PacketHeader 파싱 실패");
//                continue;
//            }
//
//            PacketType type = header.type();
//            mDispatcher.Dispatch(static_cast<size_t>(type), session, data);
//        }
//        else
//        {
//            std::this_thread::sleep_for(std::chrono::milliseconds(1));
//        }
//    }
//
//    LOG_INFO("Logic", "LogicThread 종료");
//}
//
//void ZoneThread::DisPatchPacket(int sessionId, std::span<const std::byte> data)
//{
//    PacketEx* pkt = new PacketEx(sessionId, data);
//    if (!mPacketQueue.push(pkt))
//    {
//        delete pkt;
//        LOG_ERR("Logic", "packet push failed");
//    }
//
//    LOG_INFO("OnRecv", "Recv packet - uID:{} : {}", sessionId, data.size());
//}
