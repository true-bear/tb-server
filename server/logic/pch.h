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
#include <condition_variable>

#include <boost/lockfree/queue.hpp>

#include "proto\message.pb.h"


#include "util\define.h"
#include "util\packetEx.h"
#include "util\threadManager.h"
#include "util\logger.h"
#include "util\singleton.h"
#include "util\config.h"

#include "iocp\iocpSession.h"
#include "iocp\iocpBuffer.h"
#include "iocp\iocpSocket.h"
#include "iocp\iocp.h"
#include "iocp\iocpCore.h"

static std::atomic<int> gWaitCount = 0;

#endif //PCH_H
