#include "pch.h"

enum class LogLevel : uint8_t
{
    INFO,
    WARN,
    ERR,
};

class Logger
{
public:
    static constexpr std::string_view GetLogLevelString(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERR: return "ERROR";
        default:  return "NONAME";
        }
    }

    static std::string WideToUtf8(const std::wstring& wstr)
    {
        if (wstr.empty()) return std::string();
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
        return strTo;
    }

    template<typename... Args>
    static void Log(LogLevel level, std::string_view who, const char* file, int line, std::string_view format, Args&&... args)
    {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        tm local_tm;
        localtime_s(&local_tm, &now_time_t);
        std::cout << std::format("[{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}]",
            local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday,
            local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec, ms.count());
        std::cout << std::format("[{}][{}][{}:{}] ", GetLogLevelString(level), who,
            std::filesystem::path(file).filename().string(), line);
        std::cout << std::vformat(format, std::make_format_args(args...)) << '\n';
    }

    template<typename... Args>
    static void LogW(LogLevel level, std::wstring_view who, const char* file, int line, std::wstring_view format, Args&&... args)
    {
        Log(level, WideToUtf8(std::wstring(who)), file, line, WideToUtf8(std::wstring(format)), WideToUtf8(std::to_wstring(std::forward<Args>(args)))...);
    }
};

#define LOG_INFO(who, format, ...)  Logger::Log(LogLevel::INFO, who, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_WARN(who, format, ...)  Logger::Log(LogLevel::WARN, who, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_ERR(who, format, ...) Logger::Log(LogLevel::ERR, who, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOG_INFOW(who, format, ...)  Logger::LogW(LogLevel::INFO, who, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_WARNW(who, format, ...)  Logger::LogW(LogLevel::WARN, who, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_ERRW(who, format, ...) Logger::LogW(LogLevel::ERR, who, __FILE__, __LINE__, format, ##__VA_ARGS__)