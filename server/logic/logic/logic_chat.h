#pragma once
#include "pch.h"
class Session;

void ProcessChat(Session* session, std::span<const std::byte> data);
