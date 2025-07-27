#include "pch.h"
#include "worker.h"
#include "../core.h"
#include "../iocp/iocp.h"
#include "../util/logger.h"

Worker::Worker(Core* core, std::string_view name, int index)
	: ThreadImpl(name)
	, mCore(core)
	, mIndex(index)
{
}

void Worker::Run(std::stop_token st)
{
	while (mCore->IsRunThread() && !st.stop_requested())
	{
		Iocp::IocpEvents events;
		mCore->GetIocpEvents(events, 5);

		for (int i = 0; i < events.m_eventCount; ++i)
		{
			auto& ioEvent = events.m_IoArray[i];

			if (ioEvent.lpOverlapped == nullptr)
			{
				LOG_INFO("Worker", "[{}] shutdown signal", mIndex);
				return;
			}

			unsigned long ioSize = ioEvent.dwNumberOfBytesTransferred;
			auto* over = reinterpret_cast<OverlappedIoEx*>(ioEvent.lpOverlapped);
			if (!over)
				continue;

			int sessionId = over->mUID;
			auto session = mCore->GetSession(sessionId);
			if (!session)
				continue;

			switch (over->mIOType)
			{
			case IO_TYPE::ACCEPT:
				mCore->OnAccept(sessionId, ioEvent.lpCompletionKey);
				break;
			case IO_TYPE::RECV:
				if (ioSize == 0)
					mCore->OnClose(sessionId);
				else
					mCore->OnRecv(sessionId, ioSize);
				break;
			case IO_TYPE::SEND:
				mCore->OnSend(sessionId, ioSize);
				break;
			default:
				break;
			}
		}
	}
}