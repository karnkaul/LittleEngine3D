#pragma once
#include <cstdint>
#if __MINGW32__
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#include <string>
#include <type_traits>
#include <vector>

#define ARR_SIZE(arr) sizeof(arr) / sizeof(arr[0])

namespace le
{
using u8 = std::uint8_t;
using s8 = std::int8_t;
using u16 = std::uint16_t;
using s16 = std::int16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using s32 = std::int32_t;
using s64 = std::int64_t;
using f32 = float;
using f64 = double;
using size_t = std::size_t;
using bytestream = std::vector<u8>;

template <typename Base, typename Derived>
constexpr bool isDerived()
{
	return std::is_base_of<Base, Derived>::value;
}

template <typename T>
std::string_view Typename(T const& t)
{
	static std::string_view const PREFIX = "class le::";
	static size_t const PREFIX_LEN = PREFIX.length();
	std::string_view name = typeid(t).name();
	auto idx = name.find(PREFIX);
	if (idx != std::string::npos)
	{
		name = name.substr(PREFIX_LEN, name.length() - PREFIX_LEN);
	}
	return name;
}
} // namespace le
