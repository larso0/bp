#ifndef BP_SCENE_CAMERA_H
#define BP_SCENE_CAMERA_H

#include "Node.h"

namespace bpScene
{

class Camera
{
public:
	Camera(Node* node = nullptr) : node{node} {}

	Node* getNode()
	{
		return node;
	}

	const Node* getNode() const
	{
		return node;
	}

	const glm::mat4& getViewMatrix() const
	{
		return viewMatrix;
	}

	const glm::mat4& getProjectionMatrix() const
	{
		return projectionMatrix;
	}

	void setNode(Node* n)
	{
		node = n;
	}

	void setPerspectiveProjection(float fov, float ratio, float near,
				      float far)
	{
		projectionMatrix = glm::perspective(fov, ratio, near, far);
	}

	void setOrthoProjection(float left, float right, float top,
				float bottom, float near, float far)
	{
		projectionMatrix = glm::ortho(left, right, top, bottom, near, far);
	}

	void update()
	{
		if (node)
		{
			glm::vec3 dir = quatTransform(node->getOrientation(),
						      glm::vec3(0.f, 0.f, -1.f));
			glm::vec3 up = quatTransform(node->getOrientation(),
						     glm::vec3(0.f, 1.f, 0.f));
			viewMatrix = glm::lookAt(node->getPosition(),
						 node->getPosition() + dir, up);
		} else
		{
			viewMatrix = glm::mat4();
		}
	}

private:
	Node* node;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
};

}

#endif
