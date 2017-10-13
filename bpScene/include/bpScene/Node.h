#ifndef BP_SCENE_NODE_H
#define BP_SCENE_NODE_H

#include "Math.h"
#include <vector>

namespace bpScene
{

class Node
{
public:
	Node(Node* p = nullptr) : parent(nullptr)
	{
		if (p) p->addChild(this);
	}

	virtual ~Node() = default;

	/*
	 * Update the world matrix, position and orientation.
	 */
	virtual void update();

	void setParent(Node* parent)
	{
		parent->addChild(this);
	}

	void addChild(Node* child);
	void removeChild(Node* child);

	void translate(float x, float y, float z)
	{
		translation = glm::vec3(x, y, z);
	}

	void translate(const glm::vec3& t)
	{
		translation += t;
	}

	void rotate(float angle, const glm::vec3& axis)
	{
		rotation *= glm::angleAxis(angle, axis);
	}

	void setTranslation(float x, float y, float z)
	{
		translation = glm::vec3(x, y, z);
	}

	void setTranslation(const glm::vec3& t)
	{
		translation = t;
	}

	void setRotation(float angle, const glm::vec3& axis)
	{
		rotation = glm::angleAxis(angle, axis);
	}

	Node* getParent()
	{
		return parent;
	}

	std::vector<Node*>& getChildren()
	{
		return children;
	}

	const glm::vec3& getTranslation() const
	{
		return translation;
	}

	const glm::quat& getRotation() const
	{
		return rotation;
	}

	const glm::vec3& getPosition() const
	{
		return position;
	}

	const glm::quat& getOrientation() const
	{
		return orientation;
	}

	const glm::mat4& getWorldMatrix() const
	{
		return worldMatrix;
	}

protected:
	Node* parent;
	std::vector<Node*> children;

	/*
	 * Relative local properties
	 */
	glm::vec3 translation;
	glm::quat rotation;

	/*
	 * Absolute world properties
	 */
	glm::vec3 position;
	glm::quat orientation;
	glm::mat4 worldMatrix;

};

}

#endif
