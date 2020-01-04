#include "le3d/core/ubyte.hpp"

namespace le
{
UByte UByte::lerp(UByte lhs, UByte rhs, f32 alpha)
{
	return ((1.0f - alpha) * lhs) + (alpha * rhs);
}

UByte::UByte(u8 value) : rawValue(value) {}

UByte::UByte(f32 nValue)
{
	rawValue = u8(nValue * 0xff);
}

u8 UByte::toU8() const
{
	return rawValue;
}

f32 UByte::toF32() const
{
	return rawValue / 255.0f;
}

std::string UByte::toStr() const
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