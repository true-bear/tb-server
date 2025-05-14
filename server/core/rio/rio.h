#pragma once
#include "pch.h"

class RIOCore
{
public:
    static bool Init();

    static RIO_EXTENSION_FUNCTION_TABLE Table;  // 모든 함수 포인터가 여기에 있음
};
