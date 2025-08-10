#pragma once
#include "../pch.h"
#include "../logic_dispatch.h"
#include "../logic/common_types.h"


import iocp.session;
import thread.Impl;

class LogicThread : public ThreadImpl
{
public:
    LogicThread(std::string_view name,
        SessionGetFunc getSession,
        LogicDispatch& dispatcher,
        PacketQueueT& queue,
        FreeListT& freeList
    )
        : ThreadImpl(name)
        , mGetSession(getSession)
        , mDispatcher(dispatcher)
        , mPacketQueue(queue)
        , mFreeList(freeList)
    {
    }

    void Run(std::stop_token st) override;

private:
    SessionGetFunc mGetSession;
    LogicDispatch& mDispatcher;
    PacketQueueT& mPacketQueue;
    FreeListT& mFreeList;
};
