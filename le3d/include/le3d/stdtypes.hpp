#pragma once
#include <cstdint>
#include <functional>
#if __MINGW32__
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#include <type_traits>

#define ARR_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#define SIZEOF_VEC(vec) (vec.empty() ? 0 : vec.size() * sizeof(vec[0]))

using u8 = uint8_t;
using s8 = int8_t;
using u16 = uint16_t;
using s16 = int16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s32 = int32_t;
using s64 = int64_t;
using f32 = float;
using f64 = double;
using size_t = std::size_t;
using Task = std::function<void()>;

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
