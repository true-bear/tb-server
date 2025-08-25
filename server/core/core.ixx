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

template <typename T>
concept DispatchCallable =
    requires(T t, uint64_t id, std::span<const std::byte> data) {
        { t(id, data) } -> std::same_as<void>;
};

export class Core : public Iocp, public IEventHandler, public IIoHandler
{
public:
    Core();
    virtual ~Core();

    bool            Init(const int listenPort, const int maxSession, const int workerCount);
    void            Run();
    void            Stop();
    Session*        GetSession(const std::uint64_t uID) const;

    template <DispatchCallable F>
    inline void     SetDispatchCallback(F&& callback){ mDispatchCallback = std::forward<F>(callback); }   

    bool            ConnectTo(const std::wstring& ip, const uint16_t port, const ServerRole role, const std::uint64_t logicSessionId);
private:

    bool            CreateSessionPool();
    const SOCKET&   GetListenSocket() const { return mListenSocket.GetSocket(); }
    bool            IsRunThread() const;
    void            OnAccept(const std::uint64_t sessionId, const std::uint64_t key) override;
    void            OnRecv(const std::uint64_t sessionId, const std::uint32_t size) override;
    void            OnSend(const std::uint64_t sessionId, const std::uint32_t size) override;
    void            OnClose(const std::uint64_t sessionId) override;
    void            OnConnect(const std::uint64_t sessionId) override;
    void            GetIocpEvents(IocpEvents& events, unsigned long timeout);

protected:
    std::function<void(std::uint64_t, std::span<const std::byte>)> mDispatchCallback;

private:
    SocketEx            mListenSocket;

    using SessionPool = std::unordered_map<std::uint64_t, std::unique_ptr<Session>>;
    SessionPool         mSessionPool;

    using WorkerPool =  std::vector<std::unique_ptr<Worker>>;
    WorkerPool          mWorkers;

    int                 mWorkerCnt{ 0 };

    int                 mMaxSession{ 0 };
    std::atomic<bool>   mIsRunThread{ false };

    std::mutex mSessionLock;
};