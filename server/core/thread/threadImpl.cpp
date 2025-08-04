module;

module thread.Impl;


ThreadImpl::ThreadImpl(std::string_view name)
    : mName(name)
{
}

ThreadImpl::~ThreadImpl()
{
    Stop();
}

void ThreadImpl::Start()
{
    if (mRunning.exchange(true) == false)
    {
        mThread = std::jthread([this](std::stop_token st) {
            this->Run(st);
            mRunning = false;
            });
    }
}

void ThreadImpl::Stop()
{
    mThread.request_stop();
    mRunning = false;
}

bool ThreadImpl::IsRunning() const
{
    return mRunning.load();
}

std::string_view ThreadImpl::GetName() const
{
    return mName;
}