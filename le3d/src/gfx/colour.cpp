#include "le3d/gfx/colour.hpp"
#include "le3d/core/maths.hpp"

namespace le
{
Colour const Colour::Black(0, 0, 0);
Colour const Colour::White;
Colour const Colour::Red(255, 0, 0);
Colour const Colour::Green(0, 255, 0);
Colour const Colour::Blue(0, 0, 255);
Colour const Colour::Yellow(255, 255, 0);
Colour const Colour::Magenta(255, 0, 255);
Colour const Colour::Cyan(0, 255, 255);
Colour const Colour::Transparent(0, 0, 0, 0);

Colour Colour::lerp(Colour min, Colour max, f32 alpha)
{
	if (min == max)
	{
		return min;
	}
	return ((1.0f - alpha) * min) + (alpha * max);
}

Colour::Colour(UByte r, UByte g, UByte b, UByte a) : r(r), g(g), b(b), a(a) {}

Colour& Colour::operator+=(Colour rhs)
{
	r += rhs.r;
	g += rhs.g;
	b += rhs.b;
	a += rhs.a;
	return *this;
}

Colour& Colour::operator-=(Colour rhs)
{
	r -= rhs.r;
	g -= rhs.g;
	b -= rhs.b;
	a -= rhs.a;
	return *this;
}

std::string Colour::toStr() const
{
	static constexpr size_t MAX = 1 + 3 + 1 + 3 + 1 + 3 + 1 + 3 + 1;
	std::string ret;
	ret.reserve(MAX);
	ret += "[";
	ret += r.toStr();
	ret += ", ";
	ret += g.toStr();
	ret += ", ";
	ret += b.toStr();
	ret += ", ";
	ret += a.toStr();
	ret += "]";
	return ret;
}

Colour operator+(Colour lhs, Colour rhs)
{
	return lhs += rhs;
}

Colour operator-(Colour lhs, Colour rhs)
{
	return lhs -= rhs;
}

Colour& operator*=(f32 n, Colour& c)
{
	n = maths::clamp01(n);
	c.r = n * c.r;
	c.g = n * c.g;
	c.b = n * c.b;
	c.a = n * c.a;
	return c;
}

Colour operator*(f32 n, Colour colour)
{
	return n *= colour;
}

bool operator==(Colour lhs, Colour rhs)
{
	return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}

bool operator!=(Colour lhs, Colour rhs)
{
	return !(lhs == rhs);
}
} // namespace le
