
export module iface.handler.io;

export class IIoHandler
{
public:
    virtual ~IIoHandler() = default;

    virtual void OnAccept(unsigned int sessionId, unsigned long long key) = 0;
    virtual void OnRecv(unsigned int sessionId, unsigned long size) = 0;
    virtual void OnSend(unsigned int sessionId, unsigned long size) = 0;
    virtual void OnClose(unsigned int sessionId) = 0;
};