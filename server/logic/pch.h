#ifndef PCH_H
#define PCH_H

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
#include "packetEx.h"
#include "threadManager.h"
#include "logger.h"
#include "iocp\clientSession.h"
#include "iocp\bufferEx.h"
#include "socket.h"
#include "iocp/iocp.h"
#include "core.h"
#include "logic.h"
#include "config.h"

static std::atomic<int> gWaitCount = 0;

#endif //PCH_H
