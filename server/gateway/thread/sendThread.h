#pragma once
#include <functional>
#include <memory>
#include <thread>
#include <stop_token>
#include "sendType.h"

import iocp.session;
import thread.Impl;
import thread.types;

class Session;

class SendThread : public ThreadImpl 
{
public:
    SendThread(const char* name, std::function<Session* (std::uint64_t)> getSession, GatewaySendQueue& queue);

    void Run(std::stop_token st) override;

private:
    std::function<Session* (std::uint64_t)> mGetSession;
    GatewaySendQueue& mQueue;
};
