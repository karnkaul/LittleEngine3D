#pragma once

#if defined(DEBUGGING)
#define ASSERT(predicate, errorMessage) le::assertMsg(!!(predicate), #errorMessage, __FILE__, __LINE__)
#define ASSERT_VAR(predicate, szStr) le::assertMsg(!!(predicate), szStr, __FILE__, __LINE__)
#else
#define ASSERT(disabled, _disabled)
#define ASSERT_VAR(disabled, _disabled)
#endif

namespace le
{
void assertMsg(bool expr, const char* message, const char* fileName, long lineNumber);
void debugBreak();
}
