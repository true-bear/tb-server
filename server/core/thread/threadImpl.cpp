module;
#include <windows.h>

module thread.Impl;

ThreadImpl::ThreadImpl(std::string_view name, ThreadType type)
    : mName(name), mType(type) 
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
			SetThreadName(mName);
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

void ThreadImpl::SetThreadName(std::string_view name)
{
    using SetThreadDescriptionFn = HRESULT(WINAPI*)(HANDLE, PCWSTR);
    HMODULE h = ::GetModuleHandleW(L"Kernel32.dll");
    if (!h) return;
    auto* fn = reinterpret_cast<SetThreadDescriptionFn>(
        ::GetProcAddress(h, "SetThreadDescription"));
    if (!fn) return;

    int wlen = MultiByteToWideChar(CP_UTF8, 0, name.data(), (int)name.size(), nullptr, 0);
    if (wlen <= 0) return;
    std::wstring wname(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, name.data(), (int)name.size(), wname.data(), wlen);
    fn(::GetCurrentThread(), wname.c_str());
}