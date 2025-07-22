#ifndef PCH_H
#define PCH_H

#include <vector>
#include <thread>
#include <iostream>
#include <atomic>
#include <memory>
#include <cstdint>
#include <iomanip>  
#include <string>

#define NOMINMAX // no min max 명시
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <mswsock.h>

#include <algorithm>
#include <queue>
#include <mutex>
#include <array>

#include <cstring>
#include <span>
#include <functional>
#include <cassert>
#include <format>
#include <string_view>
#include <chrono>
#include <filesystem>

#include <atlstr.h>

#include <boost/lockfree/queue.hpp>

#endif //PCH_H
