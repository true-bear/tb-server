#include "sendThread.h"
#include <chrono>
#include <iostream>
#include <span>

import iocp.session;

SendThread::SendThread(const char* name, std::function<Session* (std::uint64_t)> getSession, GatewaySendQueue& queue)
    : ThreadImpl(name, ThreadType::Gateway), mGetSession(std::move(getSession)), mQueue(queue)
{

}

void SendThread::Run(std::stop_token st)
{
    using namespace std::chrono_literals;

    while (!st.stop_requested()) 
    {
        GatewaySendNode n{};
        if (!mQueue.pop(n)) 
        {
            std::this_thread::sleep_for(50us);
            continue;
        }

        if (Session* s = mGetSession(n.sessionId)) 
        {
            std::span<const std::byte> payload(n.data.data(), static_cast<std::size_t>(n.size));
            s->SendPacket(payload);
        }
    }
}
