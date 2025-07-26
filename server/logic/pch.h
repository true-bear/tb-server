#pragma once
#include <iostream>

#include <vector>
#include <thread>
#include <atomic>
#include <memory>
#include <cstdint>
#include <iomanip>  
#include <algorithm>
#include <string>

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <mswsock.h>

#include <queue>
#include <mutex>
#include <array>
#include <typeindex>

#include <functional>

#include <format>
#include <string_view>
#include <chrono>
#include <filesystem>
#include <atlstr.h>
#include <unordered_map>

#include <boost/lockfree/queue.hpp>

#include "generated/message.pb.h"
#include "define.h"
#include "core.h"
#include "packetEx.h"
#include "threadManager.h"
#include "clientSession.h"
#include "roundBuffer.h"
#include "socket.h"
#include "iocp.h"
#include "singleton.h"
#include "logger.h"
#include <typeindex>

#include "config.h"

