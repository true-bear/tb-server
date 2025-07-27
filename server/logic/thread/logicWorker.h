#pragma once
#include "../pch.h"
#include "../logic_dispatch.h"

class LogicWorker : public ThreadImpl
{
public:
    LogicWorker(std::string_view name,
        std::function<Session* (int)> getSessionFunc,
        LogicDispatch& dispatcher,
        boost::lockfree::queue<PacketEx*>& packetQueue);

protected:
    void Run(std::stop_token st) override;

private:
    std::function<Session* (int)> mGetSession;
    LogicDispatch& mDispatcher;
    boost::lockfree::queue<PacketEx*>& mPacketQueue;
};