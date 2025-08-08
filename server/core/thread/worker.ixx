
export module thread.worker;

import thread.Impl;
import iface.handler.io;
import iface.handler.event;
import iocp;

import <stop_token>;

export class Worker : public ThreadImpl
{
public:
	[[nodiscard]] Worker(IEventHandler* eventHandler, IIoHandler* ioHandler, std::string_view name, int index);
	~Worker() override = default;

private:
	void Run(std::stop_token st);

	IEventHandler* mEventHandler{ nullptr };
	IIoHandler* mIoHandler{ nullptr };
	int mIndex;
};

