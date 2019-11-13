#include <algorithm>
#include <glm/gtx/matrix_decompose.hpp>
#include "le3d/core/transform.hpp"

namespace le
{
Transform::Transform() = default;
Transform::Transform(Transform&&) = default;
Transform& Transform::operator=(Transform&&) = default;
Transform::~Transform()
{
	if (m_pParent)
	{
		m_pParent->m_children.remove(this);
	}
	for (auto pChild : m_children)
	{
		pChild->m_pParent = nullptr;
	}
}

void Transform::setPosition(glm::vec3 position)
{
	m_position = position;
	m_bDirty = true;
}

void Transform::setOrientation(glm::quat orientation)
{
	m_orientation = orientation;
	m_bDirty = true;
}

void Transform::setScale(f32 scale)
{
	m_scale = {scale, scale, scale};
	m_bDirty = true;
}

void Transform::setScale(glm::vec3 scale)
{
	m_scale = scale;
	m_bDirty = true;
}

void Transform::setParent(Transform* pParent)
{
	if (m_pParent)
	{
		m_pParent->m_children.remove(this);
	}
	m_pParent = pParent;
	if (m_pParent)
	{
		m_pParent->m_children.push_back(this);
	}
}

glm::vec3 Transform::position() const
{
	return m_position;
}

glm::quat Transform::orientation() const
{
	return m_orientation;
}

glm::vec3 Transform::scale() const
{
	return m_scale;
}

glm::vec3 Transform::worldPos() const
{
	return glm::vec3(model()[3]);
}

glm::quat Transform::worldOrn() const
{
	glm::vec3 pos;
	glm::quat orn;
	glm::vec3 scl;
	glm::vec3 skw;
	glm::vec4 psp;
	glm::decompose(model(), scl, orn, pos, skw, psp);
	return glm::conjugate(orn);
}

glm::vec3 Transform::worldScl() const
{
	glm::vec3 pos;
	glm::quat orn;
	glm::vec3 scl;
	glm::vec3 skw;
	glm::vec4 psp;
	glm::decompose(model(), scl, orn, pos, skw, psp);
	return scl;
}

glm::mat4 Transform::model() const
{
	if (m_bDirty || m_pParent)
	{
		m_mat = m_pParent ? m_pParent->model() : glm::mat4(1.0f);
		m_mat = glm::translate(m_mat, m_position) * glm::toMat4(m_orientation);
		m_mat = glm::scale(m_mat, m_scale);
		m_bDirty = false;
	}
	return m_mat;
}
} // namespace le
