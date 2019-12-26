#pragma once
#include <functional>
#include <string>
#include "le3d/defines.hpp"

namespace le
{
#define LOG_E(msg, ...) log(le::LogLevel::Error, msg, ##__VA_ARGS__)
#define LOGIF_E(cond, msg, ...)                       \
	if (cond)                                         \
	{                                                 \
		log(le::LogLevel::Error, msg, ##__VA_ARGS__); \
	}
#define LOG_W(msg, ...) log(le::LogLevel::Warning, msg, ##__VA_ARGS__)
#define LOGIF_W(cond, msg, ...)                         \
	if (cond)                                           \
	{                                                   \
		log(le::LogLevel::Warning, msg, ##__VA_ARGS__); \
	}
#define LOG_I(msg, ...) log(le::LogLevel::Info, msg, ##__VA_ARGS__)
#define LOGIF_I(cond, msg, ...)                      \
	if (cond)                                        \
	{                                                \
		log(le::LogLevel::Info, msg, ##__VA_ARGS__); \
	}
#if defined(DEBUG_LOG)
#define LOG_D(msg, ...) log(le::LogLevel::Debug, msg, ##__VA_ARGS__)
#define LOGIF_D(cond, msg, ...)                       \
	if (cond)                                         \
	{                                                 \
		log(le::LogLevel::Debug, msg, ##__VA_ARGS__); \
	}
#else
#define LOG_D(msg, ...)
#define LOGIF_D(cond, msg, ...)
#endif

enum class LogLevel
{
	Debug = 0,
	Info,
	Warning,
	Error
};

extern std::function<void(std::string)> g_onLogStr;

void log(LogLevel level, const char* szText, ...);
} // namespace le
