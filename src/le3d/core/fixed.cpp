#include <array>
#include <iostream>
#include <math.h>
#include <string>
#include "le3d/core/fixed.hpp"

namespace le
{
const Fixed Fixed::Zero = Fixed(0, 1);
const Fixed Fixed::One = Fixed(1, 1);
const Fixed Fixed::Two = Fixed(2, 1);
const Fixed Fixed::Three = Fixed(3, 1);
const Fixed Fixed::Ten = Fixed(10, 1);
const Fixed Fixed::OneHalf = Fixed(1, 2);
const Fixed Fixed::Pi = Fixed(3.14159265359);
const Fixed Fixed::OneThird = Fixed(1, 3);
const Fixed Fixed::OneTenth = Fixed(1, 10);
const Fixed Fixed::DegToRad = Pi / 180;
const Fixed Fixed::RadToDeg = 180 / Pi;

u32 Fixed::toU32() const
{
	f64 val = toF64();
	if (val < 0)
	{
		val = -val;
	}
	u32 floor = static_cast<u32>(val);
	if ((floor * static_cast<f64>(SCALE_FACTOR)) - static_cast<f64>(value) >= 0.5)
	{
		return floor + 1;
	}
	return floor;
}

s32 Fixed::toS32() const
{
	s32 floor = static_cast<s32>(toF64());
	if ((floor * static_cast<f64>(SCALE_FACTOR)) - static_cast<f64>(value) >= 0.5)
	{
		return floor + 1;
	}
	return floor;
}

f32 Fixed::toF32() const
{
	return static_cast<f32>(value) / static_cast<f32>(SCALE_FACTOR);
}

f64 Fixed::toF64() const
{
	return static_cast<f64>(value) / static_cast<f64>(SCALE_FACTOR);
}

Fixed Fixed::power(f32 exponent) const
{
	Fixed ret = *this;
	if (exponent < 0)
	{
		ret = inverse();
		exponent = -exponent;
	}
	return Fixed(std::pow(ret.toF64(), exponent));
}

Fixed Fixed::sqrt() const
{
	return Fixed(std::sqrt(toF64()));
}

Fixed Fixed::inverse() const
{
	return Fixed(SCALE_FACTOR, value);
}

Fixed Fixed::sin() const
{
	return Fixed(std::sin(toF64()));
}

Fixed Fixed::arcSin() const
{
	f64 val = toF64();
	return Fixed(std::asin(val));
}

Fixed Fixed::cos() const
{
	return Fixed(std::cos(toF64()));
}

Fixed Fixed::arcCos() const
{
	f64 val = toF64();
	return Fixed(std::acos(val));
}

Fixed Fixed::tan() const
{
	return Fixed(std::tan(toF64()));
}

Fixed Fixed::arcTan() const
{
	return Fixed(std::atan(toF64()));
}

std::string Fixed::toStr() const
{
	std::array<char, 16> buf;
	SPRINTF(buf.data(), buf.size(), "%.2f", toF32());
	return std::string(buf.data());
}
} // namespace Core
