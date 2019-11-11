#pragma once

namespace le
{
#define logE(msg, ...) log(le::LogLevel::Error, msg, ##__VA_ARGS__)
#define logifE(cond, msg, ...)                        \
	if (cond)                                         \
	{                                                 \
		log(le::LogLevel::Error, msg, ##__VA_ARGS__); \
	}
#define logW(msg, ...) log(le::LogLevel::Warning, msg, ##__VA_ARGS__)
#define logifW(cond, msg, ...)                          \
	if (cond)                                           \
	{                                                   \
		log(le::LogLevel::Warning, msg, ##__VA_ARGS__); \
	}
#define logI(msg, ...) log(le::LogLevel::Info, msg, ##__VA_ARGS__)
#define logifI(cond, msg, ...)                       \
	if (cond)                                        \
	{                                                \
		log(le::LogLevel::Info, msg, ##__VA_ARGS__); \
	}
#if defined(DEBUGGING)
#define logD(msg, ...) log(le::LogLevel::Debug, msg, ##__VA_ARGS__)
#define logifD(cond, msg, ...)                        \
	if (cond)                                         \
	{                                                 \
		log(le::LogLevel::Debug, msg, ##__VA_ARGS__); \
	}
#else
#define logD(msg, ...)
#define logifD(cond, msg, ...)
#endif

enum class LogLevel
{
	Debug = 0,
	Info,
	Warning,
	Error
};

void log(LogLevel level, const char* szText, ...);
} // namespace le
