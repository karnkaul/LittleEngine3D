#include <sstream>
#include "le3d/engine/gfx/colour.hpp"
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

Colour::Colour(UByte r, UByte g, UByte b, UByte a) : r(r), g(g), b(b), a(a) {}

Colour Colour::lerp(Colour min, Colour max, f32 alpha)
{
	if (alpha > 0.004f)
	{
		auto lerpChannel = [&](UByte& out_l, UByte const& r) {
			if (out_l != r)
			{
				out_l = maths::lerp(out_l, r, alpha);
			}
		};
		lerpChannel(min.r, max.r);
		lerpChannel(min.g, max.g);
		lerpChannel(min.b, max.b);
		lerpChannel(min.a, max.a);
	}
	return min;
}

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

std::string Colour::toString() const
{
	std::stringstream ret;
	ret << r.toString() << ", " << g.toString() << ", " << b.toString() << ", " << a.toString();
	return ret.str();
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
	n = maths::clamp(n, 0.0f, 1.0f);
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
