#pragma once
#include <bitset>
#include <vector>
#include "le3d/stdtypes.hpp"

namespace le
{
template <size_t N>
struct TFlags
{
	std::bitset<N> flags;

	bool isSet(s32 flag) const;
	void set(s32 flag, bool bValue);
	void set(std::initializer_list<s32> flagList, bool bValue);
	void set(std::vector<s32> const& flagList, bool bValue);
};

template <size_t N>
bool TFlags<N>::isSet(s32 flag) const
{
	return flags[(size_t)flag];
}

template <size_t N>
void TFlags<N>::set(s32 flag, bool bValue)
{
	flags[(size_t)flag] = bValue;
}

template <size_t N>
void TFlags<N>::set(std::initializer_list<s32> flagList, bool bValue)
{
	for (auto flag : flagList)
	{
		flags[(size_t)flag] = bValue;
	}
}

template <size_t N>
void TFlags<N>::set(std::vector<s32> const& flagList, bool bValue)
{
	for (auto flag : flagList)
	{
		flags[(size_t)flag] = bValue;
	}
}
} // namespace le
