#pragma once
#include "pch.h"

class RIOCore
{
public:
    static bool Init();

    static RIO_EXTENSION_FUNCTION_TABLE Table;  // ��� �Լ� �����Ͱ� ���⿡ ����
};
