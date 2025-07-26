#pragma once
#include "clientSession.h";

void ProcessChat(ClientSession* session, std::span<const std::byte> data);
