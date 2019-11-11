#include <array>
#include <cmath>
#include <string>
#include "le3d/core/vector2.hpp"

namespace le
{
const Vector2 Vector2::Zero = Vector2(Fixed::Zero, Fixed::Zero);
const Vector2 Vector2::One = Vector2(Fixed::One, Fixed::One);
const Vector2 Vector2::Up = Vector2(Fixed::Zero, Fixed::One);
const Vector2 Vector2::Right = Vector2(Fixed::One, Fixed::Zero);

Vector2 Vector2::ToOrientation(Fixed degrees)
{
	Fixed rad = Fixed::DegToRad * degrees;
	return Vector2(rad.cos(), rad.sin());
}

Fixed Vector2::ToOrientation(Vector2 orientation)
{
	if (orientation.sqrMagnitude() < 0.001)
	{
		return Fixed::Zero;
	}
	orientation.normalise();
	Fixed cos = orientation.dot(Right);
	Fixed sin = orientation.dot(Up);
	Fixed degrees;
	// +/- 90
	if (cos == Fixed::Zero)
	{
		if (sin < Fixed::Zero)
		{
			degrees = -90;
		}
		else
		{
			degrees = 90;
		}
	}
	else
	{
		// Will only return [-90, 90]
		degrees = (sin / cos).arcTan() * Fixed::RadToDeg;
	}
	if (orientation.x < Fixed::Zero)
	{
		// So add 180 if facing left
		degrees += 180;
	}
	return degrees;
}

Vector2 Vector2::Rotate(Vector2 orientation, Fixed degrees)
{
	Fixed c = (degrees * Fixed::DegToRad).cos();
	Fixed s = (degrees * Fixed::DegToRad).sin();
	Fixed x = orientation.x * c - orientation.y * s;
	Fixed y = orientation.x * s + orientation.y * c;
	return {x, y};
}

Vector2 Vector2::normalised() const
{
	f64 sq = sqrMagnitude();
	if (sq == 0)
	{
		return Vector2::Zero;
	}
	auto coeff = Fixed(std::sqrt(sq)).inverse();
	return Vector2(x * coeff, y * coeff);
}

void Vector2::normalise()
{
	*this = normalised();
}

Fixed Vector2::magnitude() const
{
	return Fixed(std::sqrt(sqrMagnitude()));
}

f64 Vector2::sqrMagnitude() const
{
	return (x.toF64() * x.toF64()) + (y.toF64() * y.toF64());
}

std::string Vector2::toStr() const
{
	std::array<char, 40> buf;
	SPRINTF(buf.data(), buf.size(), "(%.2f, %.2f)", x.toF32(), y.toF32());
	return std::string(buf.data());
}
} // namespace Core
