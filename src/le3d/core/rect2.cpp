#include "le3d/core/rect2.hpp"
#include "le3d/core/maths.hpp"

namespace le
{
const Rect2 Rect2::Zero = {Vector2::Zero, Vector2::Zero};
const Rect2 Rect2::UV = {{0, 1}, {1, 0}};

Rect2 Rect2::sizeCentre(Vector2 size, Vector2 centre)
{
	Vector2 halfSize = Fixed::OneHalf * size;
	Vector2 tl = centre + Vector2(-halfSize.x, halfSize.y);
	Vector2 br = centre + Vector2(halfSize.x, -halfSize.y);
	return {tl, br};
}

Rect2 Rect2::bltr(Vector2 bottomLeft, Vector2 topRight)
{
	Vector2 tl(bottomLeft.x, topRight.y);
	Vector2 br(topRight.x, bottomLeft.y);
	return {tl, br};
}

Rect2 Rect2::tlbr(Vector2 topLeft, Vector2 bottomRight)
{
	return {topLeft, bottomRight};
}

Rect2 Rect2::sizeTL(Vector2 size, Vector2 topLeft)
{
	return {topLeft, topLeft + size};
}

Rect2 Rect2::lerp(const Rect2& min, const Rect2& max, Fixed alpha)
{
	Vector2 tl = Maths::lerp(min.tl, max.tl, alpha);
	Vector2 br = Maths::lerp(min.br, max.br, alpha);
	return Rect2{tl, br};
}

Fixed Rect2::height() const
{
	return Maths::abs(tl.y - br.y);
}

Fixed Rect2::width() const
{
	return Maths::abs(br.x - tl.x);
}

Vector2 Rect2::size() const
{
	return Vector2(width(), height());
}

Vector2 Rect2::centre() const
{
	return Fixed::OneHalf * (tl + br);
}

Vector2 Rect2::topLeft() const
{
	return tl;
}

Vector2 Rect2::topRight() const
{
	return {br.x, tl.y};
}

Vector2 Rect2::bottomLeft() const
{
	return {tl.x, br.y};
}

Vector2 Rect2::bottomRight() const
{
	return br;
}

bool Rect2::isPointIn(Vector2 point) const
{
	return point.x >= tl.x && point.x <= br.x && point.y >= br.y && point.y <= tl.y;
}
} // namespace Core
