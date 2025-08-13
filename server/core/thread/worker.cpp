module;

#include <iostream>
#include <format>

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

		auto eventRange = std::span{ events.m_IoArray, static_cast<size_t>(events.m_eventCount) };
		for (auto& ioEvent : eventRange)
		{
			if (ioEvent.lpOverlapped == nullptr)
			{
				std::cout << std::format("[{}] IOCP event without overlapped structure\n", mIndex);
				return;
			}

			unsigned long ioSize = ioEvent.dwNumberOfBytesTransferred;
			auto* over = reinterpret_cast<OverlappedIoEx*>(ioEvent.lpOverlapped);
			if (!over)
				continue;

			int sessionId = over->mUID;
			auto session = mEventHandler->GetSession(sessionId);
			if (!session)
				continue;

			switch (over->mIOType)
			{
			case IO_TYPE::CONNECT:
				mIoHandler->OnConnect(sessionId);
				break;
			case IO_TYPE::ACCEPT:
				mIoHandler->OnAccept(sessionId, ioEvent.lpCompletionKey);
				break;
			case IO_TYPE::RECV:
				if (ioSize == 0)
					mIoHandler->OnClose(sessionId);
				else
					mIoHandler->OnRecv(sessionId, ioSize);
				break;
			case IO_TYPE::SEND:
				mIoHandler->OnSend(sessionId, ioSize);
				break;
			default:
				break;
			}
		}
	}
}