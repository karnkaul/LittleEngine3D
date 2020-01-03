#include <array>
#include <iostream>
#include <math.h>
#include <string>
#include "le3d/core/fixed.hpp"

namespace le
{
Fixed const Fixed::Zero = Fixed(0, 1);
Fixed const Fixed::One = Fixed(1, 1);
Fixed const Fixed::Two = Fixed(2, 1);
Fixed const Fixed::Three = Fixed(3, 1);
Fixed const Fixed::Ten = Fixed(10, 1);
Fixed const Fixed::OneHalf = Fixed(1, 2);
Fixed const Fixed::Pi = Fixed(3.14159265359);
Fixed const Fixed::OneThird = Fixed(1, 3);
Fixed const Fixed::OneTenth = Fixed(1, 10);
Fixed const Fixed::DegToRad = Pi / 180;
Fixed const Fixed::RadToDeg = 180 / Pi;

u32 Fixed::toU32() const
{
	f64 val = toF64();
	if (val < 0)
	{
		val = -val;
	}
	u32 floor = u32(val);
	if ((floor * f64(SCALE_FACTOR)) - f64(value) >= 0.5)
	{
		return floor + 1;
	}
	return floor;
}

s32 Fixed::toS32() const
{
	s32 floor = s32(toF64());
	if ((floor * f64(SCALE_FACTOR)) - f64(value) >= 0.5)
	{
		return floor + 1;
	}
	return floor;
}

f32 Fixed::toF32() const
{
	return f32(value) / f32(SCALE_FACTOR);
}

f64 Fixed::toF64() const
{
	return f64(value) / f64(SCALE_FACTOR);
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
	std::snprintf(buf.data(), buf.size(), "%.2f", toF32());
	return std::string(buf.data());
}
} // namespace le
