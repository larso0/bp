#include <bpScene/Node.h>
#include <algorithm>

using std::remove_if;

namespace bpScene
{

void Node::update()
{
	glm::mat4 localMatrix =
		glm::translate(glm::mat4(), translation) * glm::mat4_cast(rotation);

	if (parent)
	{
		position = parent->getPosition()
			   + quatTransform(parent->getOrientation(),
					   glm::normalize(translation)) * glm::length(translation);
		orientation = parent->getOrientation() * rotation;
		worldMatrix = parent->getWorldMatrix() * localMatrix;
	} else
	{
		position = translation;
		orientation = rotation;
		worldMatrix = localMatrix;
	}

	for (Node* child : children)
	{
		child->update();
	}
}

void Node::addChild(Node* child)
{
	if (child->parent) child->parent->removeChild(child);
	children.push_back(child);
	child->parent = this;
}

void Node::removeChild(Node* child)
{
	auto pred = [child](Node* c) -> bool
	{
		if (c == child)
		{
			child->parent = nullptr;
			return true;
		}
		return false;
	};
	children.erase(remove_if(children.begin(), children.end(), pred));
}

}