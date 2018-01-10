#ifndef BP_SCENE_MATH_H
#define BP_SCENE_MATH_H

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdint>

namespace bpScene
{

glm::vec3 quatTransform(const glm::quat& q, const glm::vec3& v);

}

#endif
