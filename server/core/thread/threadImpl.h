#pragma once
#include "../interface/IThread.h"
#include "../pch.h"

class ThreadImpl : public IThread
{
public:
    ThreadImpl(std::string_view name);
    virtual ~ThreadImpl();

    virtual void Start() override;
    virtual void Stop() override;
    virtual bool IsRunning() const override;
    virtual std::string_view GetName() const override;

protected:
    virtual void Run(std::stop_token st) = 0;

private:
    std::jthread mThread;
    std::string mName;
    std::atomic<bool> mRunning{ false };
};