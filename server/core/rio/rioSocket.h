#pragma once
#include "pch.h"

class RIOSocket
{
public:
    bool Init(int port);
    SOCKET Accept();

private:
    SOCKET mListenSocket = INVALID_SOCKET;
};
