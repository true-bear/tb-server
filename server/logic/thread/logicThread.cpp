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
	mPacketQueue(packetQueue)
{
}

void LogicThread::Run(std::stop_token st)
{

    while (!st.stop_requested())
    {
        PacketEx* packet = nullptr;
        if (mPacketQueue.pop(packet))
        {
            auto session = mGetSession(packet->GetSessionId());
            if (!session)
            {
				std::cout << std::format("[{}] LogicThread: session not found for id: {}\n", GetName(), packet->GetSessionId());
                continue;
            }

            const std::span<const std::byte> data = packet->GetData();
            if (data.size() <= sizeof(uint16_t))
            {
				std::cout << std::format("[{}] LogicThread: invalid packet size: {}\n", GetName(), data.size());
                continue;
            }

            const std::byte* protoStart = data.data() + sizeof(uint16_t);
            const int protoSize = static_cast<int>(data.size() - sizeof(uint16_t));

            PacketHeader header;
            if (!header.ParseFromArray(protoStart, protoSize))
            {
				std::cout << std::format("[{}] PacketHeader 파싱 실패\n", GetName());
                continue;
            }

            PacketType type = header.type();
            mDispatcher.Dispatch(static_cast<size_t>(type), session, data.data(), data.size());
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

	std::cout << std::format("[{}] LogicThread 종료\n", GetName());
}
