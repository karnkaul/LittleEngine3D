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
#include "le3d/env/env.hpp"
#if _MSC_VER
#include "Windows.h"
#endif

namespace le
{
namespace
{
std::mutex g_logMutex;
std::list<std::string> g_logCache;
std::unordered_map<LogLevel, const char*> g_prefixes = {
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

#if defined(LOG_SOURCE_LOCATION)
void logInternal(const char* szText, const char* szFile, u64 line, LogLevel level, va_list args)
#else
void logInternal(const char* szText, const char*, u64, LogLevel level, va_list args)
#endif
{
	static std::array<char, 1024> cacheStr;
	std::lock_guard<std::mutex> lock(g_logMutex);
	std::string logText;
	logText.reserve(strlen(szText) + 32);
	logText += g_prefixes[level];
	std::vsnprintf(cacheStr.data(), cacheStr.size(), szText, args);
	logText += std::string(cacheStr.data());
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	auto pTM = TM(now);
	std::snprintf(cacheStr.data(), cacheStr.size(), " [%02d:%02d:%02d]", pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
	logText += cacheStr.data();
#if defined(LOG_SOURCE_LOCATION)
	logText += " [";
	logText += szFile;
	logText += "|";
	logText += std::to_string(line);
	logText += "]";
#endif
	logText += env::g_EOL;
	std::cout << logText;
#if _MSC_VER
	OutputDebugStringA(logText.data());
#endif
	g_logCache.emplace_back(std::move(logText));
	while (g_logCache.size() > g_logCacheSize)
	{
		g_logCache.pop_front();
	}
}
} // namespace

u32 g_logCacheSize = 128;

std::list<std::string> logCache()
{
	std::lock_guard<std::mutex> lock(g_logMutex);
	return std::move(g_logCache);
}

void log(LogLevel level, const char* szText, const char* szFile, u64 line, ...)
{
	va_list argList;
	va_start(argList, line);
	logInternal(szText, szFile, line, level, argList);
	va_end(argList);
}
} // namespace le
