#include "le3d/core/ubyte.hpp"

namespace le
{
UByte::UByte(u8 value) : rawValue(value) {}

u8 UByte::toU8() const
{
	return rawValue;
}

f32 UByte::toF32() const
{
	return (255U - rawValue) / 255.0f;
}

std::string UByte::toStr() const
{
	return std::to_string(toU8());
}

bool operator==(UByte lhs, UByte rhs)
{
	return lhs.rawValue == rhs.rawValue;
}

bool operator!=(UByte lhs, UByte rhs)
{
	return !(lhs == rhs);
}

UByte UByte::operator+=(UByte rhs)
{
	rawValue = static_cast<u8>(static_cast<s16>(rawValue) + static_cast<s16>(rhs.rawValue));
	return *this;
}

UByte operator+(UByte lhs, UByte rhs)
{
	return lhs += rhs;
}

UByte UByte::operator-=(UByte rhs)
{
	rawValue = static_cast<u8>(static_cast<s16>(rawValue) - static_cast<s16>(rhs.rawValue));
	return *this;
}

UByte operator-(UByte lhs, UByte rhs)
{
	return lhs -= rhs;
}

UByte UByte::operator*=(UByte rhs)
{
	rawValue = static_cast<u8>(static_cast<s16>(rawValue) * static_cast<s16>(rhs.rawValue));
	return *this;
}

UByte operator*(UByte lhs, UByte rhs)
{
	return lhs *= rhs;
}

UByte UByte::operator/=(UByte rhs)
{
	rawValue = static_cast<u8>(static_cast<s16>(rawValue) * static_cast<s16>(rhs.rawValue));
	return *this;
}

UByte operator/(UByte lhs, UByte rhs)
{
	return lhs /= rhs;
}

UByte operator*(f32 lhs, UByte rhs)
{
	return UByte(static_cast<u8>(static_cast<f32>(rhs.rawValue) * lhs));
}
} // namespace Core
