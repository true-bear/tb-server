#pragma once
#include "../pch.h"
class IThread
{
public:
    virtual ~IThread() = default;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual bool IsRunning() const = 0;
    virtual std::string_view GetName() const = 0;
};
