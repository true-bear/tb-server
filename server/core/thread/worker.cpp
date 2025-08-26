module;

#include <iostream>
#include <format>
#include <ranges>
#include <tuple>

module thread.worker;

import thread.Impl;
import iface.handler.io;
import iface.handler.event;
import iocp;

import <stop_token>;
import <span>;

Worker::Worker(IEventHandler* eventHandler, IIoHandler* ioHandler, std::string_view name, int index, ThreadType type)
	: ThreadImpl(name, type)
	, mEventHandler(eventHandler)
	, mIoHandler(ioHandler)
	, mIndex(index) 
{
}

void Worker::Run(std::stop_token st)
{
    while (!st.stop_requested())
    {
        IocpEvents events;
        mEventHandler->GetIocpEvents(events, 5);

        std::span events_view{ events.m_IoArray, static_cast<std::size_t>(events.m_eventCount) };

        auto pipeline =
            events_view
            | std::views::take_while([](const auto& e) { return e.lpOverlapped != nullptr; })
            | std::views::transform([](const auto& e) {
            auto* over = reinterpret_cast<OverlappedIoEx*>(e.lpOverlapped);
            return std::tuple{ over, e.dwNumberOfBytesTransferred, e.lpCompletionKey };
                });

        for (auto [over, ioSize, compKey] : pipeline)
        {
            const std::uint64_t sessionId = over->mUID;
            if (auto* session = mEventHandler->GetSession(sessionId); !session)
                continue;

            switch (over->mIOType)
            {
            case IO_TYPE::CONNECT: 
                mIoHandler->OnConnect(sessionId); 
                break;
            case IO_TYPE::ACCEPT:  
                mIoHandler->OnAccept(sessionId, compKey); 
                break;
            case IO_TYPE::RECV:
                (ioSize == 0) ? mIoHandler->OnClose(sessionId): mIoHandler->OnRecv(sessionId, ioSize);
                break;
            case IO_TYPE::SEND:    
                mIoHandler->OnSend(sessionId, ioSize); 
                break;
            default: break;
            }
        }
    }
}