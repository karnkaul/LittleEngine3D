#pragma once
#include <list>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "le3d/core/stdtypes.hpp"

namespace le
{
class Transform final
{
private:
	mutable glm::mat4 m_mat = glm::mat4(1.0f);
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::quat m_orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 m_scale = glm::vec3(1.0f);
	std::list<Transform*> m_children;
	Transform* m_pParent = nullptr;
	mutable bool m_bDirty = false;

public:
	Transform();
	Transform(Transform&&) noexcept;
	Transform& operator=(Transform&&) noexcept;
	~Transform();

public:
	Transform& setPosition(glm::vec3 position);
	Transform& setOrientation(glm::quat orientation);
	Transform& setScale(f32 scale);
	Transform& setScale(glm::vec3 scale);
	Transform& setParent(Transform* pParent);

	glm::vec3 position() const;
	glm::quat orientation() const;
	glm::vec3 scale() const;
	bool isIsotropic() const;

	glm::vec3 worldPos() const;
	glm::quat worldOrn() const;
	glm::vec3 worldScl() const;

	glm::mat4 model() const;
	glm::mat4 normalModel() const;
};
} // namespace le
