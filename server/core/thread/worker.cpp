#include "../pch.h"
#include "worker.h"

Worker::Worker(IEventHandler* eventHandler, IIoHandler* ioHandler,std::string_view name, int index)
	: ThreadImpl(name)
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

		for (int i = 0; i < events.m_eventCount; ++i)
		{
			auto& ioEvent = events.m_IoArray[i];

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