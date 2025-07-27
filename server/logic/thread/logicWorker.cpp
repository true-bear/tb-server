#include  "pch.h"
#include "logicWorker.h"
#include "../logic_dispatch.h"

LogicWorker::LogicWorker(
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

void LogicWorker::Run(std::stop_token st)
{

    while (!st.stop_requested())
    {
        PacketEx* packet = nullptr;
        if (mPacketQueue.pop(packet))
        {
            auto session = mGetSession(packet->GetSessionId());
            if (!session)
            {
                LOG_ERR("Logic", "session nullptr");
                continue;
            }

            const std::span<const std::byte> data = packet->GetData();
            if (data.size() <= sizeof(uint16_t))
            {
                LOG_ERR("Logic", "invalid packet size");
                continue;
            }

            const std::byte* protoStart = data.data() + sizeof(uint16_t);
            const int protoSize = static_cast<int>(data.size() - sizeof(uint16_t));

            PacketHeader header;
            if (!header.ParseFromArray(protoStart, protoSize))
            {
                LOG_ERR("Logic", "PacketHeader 파싱 실패");
                continue;
            }

            PacketType type = header.type();
            mDispatcher.Dispatch(static_cast<size_t>(type), session, data);
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    LOG_INFO("Logic", "LogicThread 종료");
}
