#pragma once
#include <bitset>
#include <vector>
#include "le3d/core/stdtypes.hpp"

namespace le
{
template <size_t N, typename Enum>
struct TFlags
{
	std::bitset<N> bits;

	bool isSet(Enum flag) const;
	void set(Enum flag, bool bValue);
	void set(std::initializer_list<Enum> flagList, bool bValue);
	void set(std::vector<Enum> const& flagList, bool bValue);
	void set(bool bValue);
};

template <size_t N, typename Enum>
bool TFlags<N, Enum>::isSet(Enum flag) const
{
	return bits[(size_t)flag];
}

template <size_t N, typename Enum>
void TFlags<N, Enum>::set(Enum flag, bool bValue)
{
	bits[(size_t)flag] = bValue;
}

template <size_t N, typename Enum>
void TFlags<N, Enum>::set(std::initializer_list<Enum> flagList, bool bValue)
{
	for (auto flag : flagList)
	{
		bits[(size_t)flag] = bValue;
	}
}

template <size_t N, typename Enum>
void TFlags<N, Enum>::set(std::vector<Enum> const& flagList, bool bValue)
{
	for (auto flag : flagList)
	{
		bits[(size_t)flag] = bValue;
	}
}

template <size_t N, typename Enum>
void TFlags<N, Enum>::set(bool bValue)
{
	bValue ? bits.set() : bits.reset();
}
} // namespace le
