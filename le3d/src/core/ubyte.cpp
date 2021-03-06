#include "le3d/core/ubyte.hpp"

namespace le
{
UByte::UByte(u8 value) noexcept : rawValue(value) {}

UByte::UByte(f32 nValue) noexcept
{
	rawValue = u8(nValue * 255.0f);
}

UByte::UByte(std::string_view hex)
{
	std::string str;
	str += hex;
	while (str.length() < 2)
	{
		str += "f";
	}
	rawValue = (u8)stoul(str, nullptr, 16);
}

u8 UByte::toU8() const
{
	return rawValue;
}

f32 UByte::toF32() const
{
	return rawValue / 255.0f;
}

std::string UByte::toString() const
{
	return std::to_string(rawValue);
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
	rawValue = u8(s16(rawValue) + s16(rhs.rawValue));
	return *this;
}

UByte operator+(UByte lhs, UByte rhs)
{
	return lhs += rhs;
}

UByte UByte::operator-=(UByte rhs)
{
	rawValue = u8(s16(rawValue) - s16(rhs.rawValue));
	return *this;
}

UByte operator-(UByte lhs, UByte rhs)
{
	return lhs -= rhs;
}

UByte UByte::operator*=(UByte rhs)
{
	rawValue = u8(s16(rawValue) * s16(rhs.rawValue));
	return *this;
}

UByte operator*(UByte lhs, UByte rhs)
{
	return lhs *= rhs;
}

UByte UByte::operator/=(UByte rhs)
{
	rawValue = u8(s16(rawValue) * s16(rhs.rawValue));
	return *this;
}

UByte operator/(UByte lhs, UByte rhs)
{
	return lhs /= rhs;
}

UByte operator*(f32 lhs, UByte rhs)
{
	return UByte(u8(f32(rhs.rawValue) * lhs));
}
} // namespace le
