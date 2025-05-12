#pragma once
#include "pch.h"
class ClientSession;
void ProcessChat(ClientSession* session, const char* data, uint16_t size);
