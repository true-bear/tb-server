#pragma once

#ifndef NOMINMAX
#define NOMINMAX        // ← Windows의 min/max 매크로 비활성화
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <iostream>

#include <vector>
#include <thread>
#include <atomic>
#include <memory>
#include <iomanip>  
#include <algorithm>
#include <string>
#include <format>
#include <span>
#include <queue>
#include <mutex>
#include <typeindex>

#include <functional>

#include <string_view>
#include <chrono>
#include <filesystem>

#include <atlstr.h>
#include <unordered_map>


#include "generated/message.pb.h"


