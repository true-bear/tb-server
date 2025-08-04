#pragma once

import thread.Impl;
import iocp;

import thread.Impl;
class ICore;

class Worker : public ThreadImpl
{
public:
    Worker(ICore* coreDelegate, std::string_view name, int index);
    ~Worker() override = default;

private:
    void Run(std::stop_token st);

    ICore* mCore{ nullptr };
    int mIndex{ 0 };
};

