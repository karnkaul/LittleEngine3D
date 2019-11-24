#include <array>
#include <ctime>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include "le3d/stdtypes.hpp"
#include "le3d/core/log.hpp"
#if _MSC_VER
#include "Windows.h"
#endif

namespace le
{
namespace
{
std::mutex logMutex;
std::string cache;
std::unordered_map<LogLevel, const char*> prefixes = {
	{LogLevel::Debug, "[D] "}, {LogLevel::Info, "[I] "}, {LogLevel::Warning, "[W] "}, {LogLevel::Error, "[E] "}};

std::tm* TM(const std::time_t& time)
{
#if _MSC_VER
	static std::tm tm;
	localtime_s(&tm, &time);
	return &tm;
#else
	return localtime(&time);
#endif
}

void logInternal(const char* szText, LogLevel level, va_list args)
{
	static std::array<char, 1024> cacheStr;
	std::lock_guard<std::mutex> lock(logMutex);
	cache.clear();
	cache += prefixes[level];
	vsnprintf(cacheStr.data(), cacheStr.size(), szText, args);
	cache += std::string(cacheStr.data());
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	auto pTM = TM(now);
	snprintf(cacheStr.data(), cacheStr.size(), " [%02d:%02d:%02d]", pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
	cache += std::string(cacheStr.data());
	cache += "\n";

	std::cout << cache;

#if _MSC_VER
	OutputDebugStringA(cache.data());
#endif
	if (g_onLogStr)
	{
		g_onLogStr(std::move(cache));
	}
}
} // namespace

std::function<void(std::string)> g_onLogStr;

void log(LogLevel level, const char* szText, ...)
{
	va_list argList;
	va_start(argList, szText);
	logInternal(szText, level, argList);
	va_end(argList);
}
} // namespace le
