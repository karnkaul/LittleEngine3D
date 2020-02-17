#pragma once
#include <string>
#include "le3d/core/log.hpp"
#include "le3d/core/time.hpp"

namespace le
{
struct Profiler
{
	std::string id;
	LogLevel level = LogLevel::Debug;
	Time dt;

	explicit Profiler(std::string_view id, LogLevel level = LogLevel::Debug);
	virtual ~Profiler();
};
} // namespace le
