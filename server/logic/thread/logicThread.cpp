#include "../pch.h"
#include "logicThread.h"
#include "../logic_dispatch.h"
#include "../logic/common_types.h"
#include <thread>
#include <chrono>

void LogicThread::Run(std::stop_token st)
{
    while (!st.stop_requested())
    {
        PacketNode* node = nullptr;
        if (!mPacketQueue.pop(node))
        {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            continue;
        }

        Session* session = mGetSession(node->sessionId);
        if (session)
        {
            std::span<const std::byte> payload{ node->data.data(), node->size };

            switch (node->type) 
            {
                case static_cast<int>(PacketType::CHAT):
                    mDispatcher.Dispatch(static_cast<size_t>(static_cast<int>(PacketType::CHAT)),
                        session, payload.data(), payload.size());
                    break;
                default: break;
            }
        }

        mFreeList.push(node);
    }
}
