#pragma once
#include <cstdint>
#include <cstddef>
#if defined(__MINGW32__)
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#include <string>
#include <type_traits>
#include <vector>
#if defined(__GNUG__)
#include <cxxabi.h>
#endif

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
using bytearray = std::vector<std::byte>;

template <typename... Ts>
constexpr bool alwaysFalse = false;

template <typename... Ts>
constexpr bool alwaysTrue = true;

template <typename Base, typename Derived>
constexpr bool isDerived()
{
	return std::is_base_of<Base, Derived>::value;
}

template <typename T>
std::string demangle(std::string_view typeName)
{
	std::string ret(typeName);
#if defined(__GNUG__)
	s32 status = -1;
	char* szRes = abi::__cxa_demangle(typeName.data(), nullptr, nullptr, &status);
	if (status == 0)
	{
		ret = szRes;
	}
	std::free(szRes);
#else
	constexpr std::string_view CLASS = "class ";
	constexpr std::string_view STRUCT = "struct ";
	constexpr size_t CLASS_LEN = CLASS.length();
	constexpr size_t STRUCT_LEN = STRUCT.length();
	auto idx = ret.find(CLASS);
	if (idx == 0)
	{
		ret = ret.substr(CLASS_LEN);
	}
	idx = ret.find(STRUCT);
	if (idx == 0)
	{
		ret = ret.substr(STRUCT_LEN);
	}
#endif
	return ret;
}

template <typename T>
std::string typeName(T const& t)
{
	return demangle<T>(typeid(t).name());
}

template <typename T>
std::string typeName()
{
	return demangle<T>(typeid(T).name());
}

template <typename Type>
struct Span
{
	Type* pData;
	size_t extent;

	constexpr Span() noexcept : pData(nullptr), extent(0) {}
	constexpr Span(Type* pData, size_t extent) noexcept : pData(pData), extent(extent) {}

	template <typename Container>
	constexpr Span(Container& container) noexcept : pData(container.empty() ? nullptr : &container.front()), extent(container.size())
	{
	}
};
} // namespace le
