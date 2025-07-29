#pragma once
#include "threadImpl.h"
#include "../iocp/session.h"
#include "../iocp/iocp.h"

class Core;

class Worker : public ThreadImpl
{
public:
	Worker(Core* core, std::string_view name, int index);
	~Worker() override = default;

private:
	void Run(std::stop_token st);

	Core* mCore;
	int mIndex;
};

