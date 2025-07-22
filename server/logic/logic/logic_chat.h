#pragma once
#include "pch.h"
class IocpSession;

void ProcessChat(IocpSession* session, const char* data, uint16_t size, uint32_t packetId);
