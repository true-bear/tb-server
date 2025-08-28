module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>   // DWORD, ULONG_PTR
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>

#include <iostream>
#include <format>
#include <ranges>
#include <tuple>

module thread.worker;

import thread.Impl;
import iface.handler.io;
import iface.handler.event;
import iocp;
import iocp.session;

import <stop_token>;
import <span>;

using completion_t = std::tuple<OverlappedIoEx*, DWORD, ULONG_PTR>;

static constexpr auto has_overlapped = [](const auto& e) noexcept { return e.lpOverlapped != nullptr; };

static constexpr auto to_completion = [](const auto& e) noexcept -> completion_t {
    auto* over = reinterpret_cast<OverlappedIoEx*>(e.lpOverlapped);
    return { over, e.dwNumberOfBytesTransferred, e.lpCompletionKey };
    };

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

        auto pipeline = events_view | std::views::take_while(has_overlapped) | std::views::transform(to_completion);

        for (auto [over, ioSize, compKey] : pipeline)
        {
            if (auto* session = mEventHandler->GetSession(over->mUID); !session)
                continue;

            switch (over->mIOType)
            {
            case IO_TYPE::CONNECT: 
                mIoHandler->OnConnect(over->mUID);
                break;
            case IO_TYPE::ACCEPT:  
                mIoHandler->OnAccept(over->mUID, compKey);
                break;
            case IO_TYPE::RECV:
                (ioSize == 0) ? mIoHandler->OnClose(over->mUID): mIoHandler->OnRecv(over->mUID, ioSize);
                break;
            case IO_TYPE::SEND:    
                mIoHandler->OnSend(over->mUID, ioSize);
                break;
            default: break;
            }
        }
    }
}