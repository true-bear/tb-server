export module core.engine;

import common.define;

import iface.handler.io;
import iface.handler.event;

import iocp.socket;
import iocp.session;
import iocp;

import thread.worker;

import <unordered_map>;
import <vector>;
import <memory>;
import <atomic>;
import <mutex>;
import <functional>;
import <span>;

export using DispatchFn = std::function<void(unsigned int, std::span<const std::byte>)>;

export class Core : public Iocp, public IEventHandler, public IIoHandler
{
public:
    [[nodiscard]] Core();
    virtual ~Core();

    bool Init(const int maxSession);
    void Run();
    void Stop();
    bool CreateSessionPool();
    Session* GetSession(const unsigned int uID) const;
    const SOCKET& GetListenSocket() const { return mListenSocket.GetSocket(); }
    bool IsRunThread() const;
    
    void SetDispatchCallback(DispatchFn callback);

    void OnRecv(unsigned int uID, unsigned long ioSize) override;
    void OnSend(unsigned int uID, unsigned long ioSize) override;
    void OnAccept(unsigned int uID, unsigned long long completeKey) override;
    void OnClose(unsigned int uID) override;
    void OnConnect(unsigned int uID) override;

    void GetIocpEvents(IocpEvents& events, unsigned long timeout);

    bool ConnectTo(const std::wstring& ip, uint16_t port, ServerRole role, unsigned& outSessionId);

protected:
    std::function<void(unsigned int, std::span<const std::byte>)> mDispatchCallback;

private:
    SocketEx            mListenSocket;

    using SessionPool = std::unordered_map<unsigned int, std::unique_ptr<Session>>;
    SessionPool         mSessionPool;

    using WorkerPool =  std::vector<std::unique_ptr<Worker>>;
    WorkerPool          mWorkers;

    int                 mWorkerCnt{ 0 };

    int                 mMaxSession{ 0 };
    std::atomic<bool>   mIsRunThread{ false };

    std::mutex mSessionLock;
};
