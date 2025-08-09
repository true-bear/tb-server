#include "../pch.h"
#include "logicThread.h"
#include "../logic_dispatch.h"

import iocp.session;
import thread.Impl;

LogicThread::LogicThread(
    std::string_view name,
    std::function<Session* (int)> getSessionFunc,
    LogicDispatch& dispatcher,
    boost::lockfree::queue<PacketEx*>& packetQueue)
    : ThreadImpl(name),
    mGetSession(getSessionFunc),
    mDispatcher(dispatcher),
	mPacketQueue(&packetQueue)
{
}

void LogicThread::Run(std::stop_token st)
{
    while (!st.stop_requested())
    {
        PacketEx* packet = nullptr;
        if (!mPacketQueue->pop(packet))
        {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            continue;
        }

        auto pkt = std::unique_ptr<PacketEx>(packet);

        auto session = mGetSession(pkt->GetSessionId());
        if (!session)
            continue;
        
        const std::span<const std::byte> data = pkt->GetData();
        if (data.size() <= sizeof(uint16_t))
            continue;
        
        const std::byte* protoStart = data.data() + sizeof(uint16_t);
        const int protoSize = static_cast<int>(data.size() - sizeof(uint16_t));

        PacketHeader header;
        if (!header.ParseFromArray(protoStart, protoSize))
            continue;

        PacketType type = header.type();
        mDispatcher.Dispatch(static_cast<size_t>(type), session, data.data(), data.size());
    }

    std::cout << std::format("[{}] LogicThread Á¾·á\n", GetName());
}

