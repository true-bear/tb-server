#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mswsock.h>

#include <vector>
#include <thread>
#include <iostream>
#include <atomic>
#include <memory>
#include <cstdint>
#include <iomanip>  
#include <algorithm>
#include <string>
#include <string_view>
#include <format>
#include <filesystem>
#include <typeindex>
#include <queue>
#include <mutex>
#include <array>
#include <span>
#include <functional>
#include <stop_token>
#include <chrono>

#include <atlstr.h>
#include <boost/lockfree/queue.hpp>

#include "define.h"
