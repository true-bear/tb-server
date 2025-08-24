module;
#include <cstdint>

export module iface.handler.event;

import iocp.session;
import iocp;

export class IEventHandler
{
public:
	[[nodiscard]] IEventHandler() = default;
    virtual ~IEventHandler() = default;

    virtual void GetIocpEvents(IocpEvents& outEvents, unsigned long timeout) = 0;
    virtual Session* GetSession(const std::uint64_t uid) const = 0;
};