#pragma once
#include <bitset>
#include "le3d/stdtypes.hpp"

namespace le
{
template <size_t N>
struct Flags
{
	std::bitset<N> flags;

	bool isSet(s32 flag) const;
	void set(s32 flag, bool bValue);
};

template <size_t N>
bool Flags<N>::isSet(s32 flag) const
{
	return flags[(size_t)flag];
}

template <size_t N>
void Flags<N>::set(s32 flag, bool bValue)
{
	flags[(size_t)flag] = bValue;
}
} // namespace le
