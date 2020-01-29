#pragma once
#include <functional>
#include <deque>
#include <string>
#include "le3d/core/stdtypes.hpp"

/**
 * Variable     : DEBUG_LOG
 * Description  : Used to enable LOG_D and LOGIF_D macros (LogLevel::Debug)
 */
#if defined(DEBUGGING)
#if !defined(DEBUG_LOG)
#define DEBUG_LOG
#endif
#endif

#define LOG(level, msg, ...) le::log(level, msg, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGIF(cond, level, msg, ...)                            \
	if (cond)                                                   \
	{                                                           \
		le::log(level, msg, __FILE__, __LINE__, ##__VA_ARGS__); \
	}
#define LOG_E(msg, ...) log(le::LogLevel::Error, msg, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGIF_E(cond, msg, ...)                                               \
	if (cond)                                                                 \
	{                                                                         \
		le::log(le::LogLevel::Error, msg, __FILE__, __LINE__, ##__VA_ARGS__); \
	}
#define LOG_W(msg, ...) log(le::LogLevel::Warning, msg, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGIF_W(cond, msg, ...)                                                 \
	if (cond)                                                                   \
	{                                                                           \
		le::log(le::LogLevel::Warning, msg, __FILE__, __LINE__, ##__VA_ARGS__); \
	}
#define LOG_I(msg, ...) log(le::LogLevel::Info, msg, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGIF_I(cond, msg, ...)                                              \
	if (cond)                                                                \
	{                                                                        \
		le::log(le::LogLevel::Info, msg, __FILE__, __LINE__, ##__VA_ARGS__); \
	}
#if defined(DEBUG_LOG)
#define LOG_D(msg, ...) log(le::LogLevel::Debug, msg, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGIF_D(cond, msg, ...)                                               \
	if (cond)                                                                 \
	{                                                                         \
		le::log(le::LogLevel::Debug, msg, __FILE__, __LINE__, ##__VA_ARGS__); \
	}
#else
#define LOG_D(msg, ...)
#define LOGIF_D(cond, msg, ...)
#endif

namespace le
{
enum class LogLevel
{
	Debug = 0,
	Info,
	Warning,
	Error
};

void log(LogLevel level, char const* szText, char const* szFile, u64 line, ...);

inline u32 g_logCacheSize = 128;
std::deque<std::string> logCache();
} // namespace le
