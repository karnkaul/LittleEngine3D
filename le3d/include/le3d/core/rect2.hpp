#pragma once
#include <glm/glm.hpp>
#include "le3d/core/std_types.hpp"

namespace le
{
struct Rect2
{
	glm::vec2 bl = glm::vec2(0.0f);
	glm::vec2 tr = glm::vec2(0.0f);

	glm::vec2 size() const;
	glm::vec2 centre() const;
	glm::vec2 tl() const;
	glm::vec2 br() const;
	f32 aspect() const;

	static Rect2 sizeCentre(glm::vec2 const& size, glm::vec2 const& centre = glm::vec2(0.0f));
};
} // namespace le
