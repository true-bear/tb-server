#pragma once

import thread.Impl;
import iface.handler.io;
import iface.handler.event;
import iocp;

class Core;

class Worker : public ThreadImpl
{
public:
	Worker(IEventHandler* eventHandler, IIoHandler* ioHandler, std::string_view name, int index);
	~Worker() override = default;

private:
	void Run(std::stop_token st);

	IEventHandler* mEventHandler{ nullptr };
	IIoHandler* mIoHandler{ nullptr };
	Core* mCore;
	int mIndex;
};

