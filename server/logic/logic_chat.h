#pragma once
#include "pch.h"
class ClientSession;

void ProcessChat(ClientSession* session, std::span<const std::byte> data);
