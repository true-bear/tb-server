#pragma once
#include "../pch.h"
#include "../logic_dispatch.h"

import iocp.session;
import thread.Impl;

class LogicThread : public ThreadImpl
{
public:
    [[nodiscard]] LogicThread(std::string_view name,
        std::function<Session* (int)> getSessionFunc,
        LogicDispatch& dispatcher,
        boost::lockfree::queue<PacketEx*>& packetQueue);

protected:
    void Run(std::stop_token st) override;

private:
    std::function<Session* (int)> mGetSession;
    LogicDispatch& mDispatcher;
    boost::lockfree::queue<PacketEx*>* mPacketQueue;
};