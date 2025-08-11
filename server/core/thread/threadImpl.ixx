module;
export module thread.Impl;

import iface.thread;
import thread.types;

import <string_view>;
import <stop_token>;
import <stop_token>;
import <thread>;
import <atomic>;
import <string>;

export class ThreadImpl : public IThread
{
public:
    ThreadImpl(std::string_view name, ThreadType type);
    virtual ~ThreadImpl();

    void SetThreadName(std::string_view name);

    virtual void Start() override;
    virtual void Stop() override;
    virtual bool IsRunning() const override;
    virtual std::string_view GetName() const override;

protected:
    virtual void Run(std::stop_token st) = 0;

private:
    std::jthread        mThread;
    std::string         mName;
	int                 mIndex{ 0 };
    std::atomic<bool>   mRunning{ false };
    ThreadType          mType{ ThreadType::Unknown };
};