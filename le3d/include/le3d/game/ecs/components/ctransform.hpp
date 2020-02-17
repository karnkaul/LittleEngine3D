#pragma once
#include "le3d/core/transform.hpp"
#include "le3d/game/ecs/component.hpp"

namespace le
{
class CTransform : public Component
{
public:
	Transform m_transform;
};
} // namespace le
