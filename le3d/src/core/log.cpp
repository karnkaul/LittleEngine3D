#include <array>
#include <ctime>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <sstream>
#include <unordered_map>
#include "le3d/defines.hpp"
#include "le3d/core/std_types.hpp"
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
std::deque<std::string> g_logCache;
std::unordered_map<LogLevel, char const*> g_prefixes = {
	{LogLevel::Debug, "[D] "}, {LogLevel::Info, "[I] "}, {LogLevel::Warning, "[W] "}, {LogLevel::Error, "[E] "}};

std::tm* TM(std::time_t const& time)
{
#if _MSC_VER
	static std::tm tm;
	localtime_s(&tm, &time);
	return &tm;
#else
	return localtime(&time);
#endif
}

void logInternal(char const* szText, [[maybe_unused]] char const* szFile, [[maybe_unused]] u64 line, LogLevel level, va_list args)
{
	static std::array<char, 1024> cacheStr;
	std::lock_guard<std::mutex> lock(g_logMutex);
	std::stringstream logText;
	logText << g_prefixes.at(level);
	std::vsnprintf(cacheStr.data(), cacheStr.size(), szText, args);
	logText << cacheStr.data();
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	auto pTM = TM(now);
	std::snprintf(cacheStr.data(), cacheStr.size(), " [%02d:%02d:%02d]", pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
	logText << cacheStr.data();
#if defined(LE3D_LOG_SOURCE_LOCATION)
	logText << "[" << std::filesystem::path(szFile).generic_string() << "|" << line << "]";
#endif
	logText << env::g_EOL;
	auto logStr = logText.str();
	std::cout << logStr;
#if _MSC_VER
	OutputDebugStringA(logStr.data());
#endif
	g_logCache.push_back(std::move(logStr));
	while (g_logCache.size() > g_logCacheSize)
	{
		g_logCache.pop_front();
	}
}
} // namespace

std::deque<std::string> logCache()
{
	std::lock_guard<std::mutex> lock(g_logMutex);
	return std::move(g_logCache);
}

void log(LogLevel level, char const* szText, char const* szFile, u64 line, ...)
{
	va_list argList;
	va_start(argList, line);
	logInternal(szText, szFile, line, level, argList);
	va_end(argList);
}
} // namespace le
