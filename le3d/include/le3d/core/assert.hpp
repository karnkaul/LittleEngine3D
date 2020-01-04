#pragma once
#include "le3d/defines.hpp"

#if defined(ASSERTS)
#define ASSERT(predicate, errorMessage) le::assertMsg(!!(predicate), #errorMessage, __FILE__, __LINE__)
#define ASSERT_VAR(predicate, szStr) le::assertMsg(!!(predicate), szStr, __FILE__, __LINE__)
#else
#define ASSERT(disabled, _disabled)
#define ASSERT_VAR(disabled, _disabled)
#endif

namespace le
{
void assertMsg(bool expr, char const* message, char const* fileName, long lineNumber);
void debugBreak();
} // namespace le