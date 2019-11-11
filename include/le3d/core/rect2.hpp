#pragma once
#include "vector2.hpp"

namespace le
{
// \brief Specifies an axis-aligned rectangle via bottom-left and top-right points
struct Rect2
{
public:
	Vector2 tl;
	Vector2 br;

public:
	static const Rect2 Zero;
	static const Rect2 UVs;
	static Rect2 sizeCentre(Vector2 size, Vector2 centre = Vector2::Zero);
	static Rect2 bltr(Vector2 bottomLeft, Vector2 topRight);
	static Rect2 tlbr(Vector2 topLeft, Vector2 bottomRight);
	static Rect2 sizeTL(Vector2 size, Vector2 topLeft);
	static Rect2 lerp(const Rect2& min, const Rect2& max, Fixed alpha);

	Rect2() = default;
	Rect2(const Rect2&) = default;
	Rect2(Rect2&&) = default;
	Rect2& operator=(const Rect2&) = default;
	Rect2& operator=(Rect2&&) = default;

	Fixed height() const;
	Fixed width() const;
	Vector2 size() const;
	Vector2 centre() const;
	Vector2 topLeft() const;
	Vector2 topRight() const;
	Vector2 bottomLeft() const;
	Vector2 bottomRight() const;

	bool isPointIn(Vector2 point) const;
};
} // namespace Core
