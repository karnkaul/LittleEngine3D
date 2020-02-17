#include "le3d/core/rect2.hpp"

namespace le
{
glm::vec2 Rect2::size() const
{
	return {tr.x - bl.x, tr.y - bl.y};
}

glm::vec2 Rect2::centre() const
{
	return {(tr.x + bl.x) * 0.5f, (tr.y + bl.y) * 0.5f};
}

glm::vec2 Rect2::tl() const
{
	return {bl.x, tr.y};
}

glm::vec2 Rect2::br() const
{
	return {tr.x, bl.y};
}

f32 Rect2::aspect() const
{
	auto const s = size();
	return s.x / s.y;
}

Rect2 Rect2::sizeCentre(glm::vec2 const& size, glm::vec2 const& centre)
{
	Rect2 ret;
	ret.bl = centre - size * 0.5f;
	ret.tr = centre + size * 0.5f;
	return ret;
}
} // namespace le
