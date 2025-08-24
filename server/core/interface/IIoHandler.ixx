module;
#include <cstdint>

export module iface.handler.io;

export class IIoHandler
{
public:
	[[nodiscard]] IIoHandler() = default;
    virtual ~IIoHandler() = default;

    virtual void OnAccept(const std::uint64_t sessionId, const std::uint64_t key) = 0;
    virtual void OnRecv(const std::uint64_t sessionId, const std::uint32_t size) = 0;
    virtual void OnSend(const std::uint64_t sessionId, const std::uint32_t size) = 0;
    virtual void OnClose(const std::uint64_t sessionId) = 0;
	virtual void OnConnect(const std::uint64_t sessionId) = 0;
};