#ifndef SNAKEBITE_GAME_OBJECT_H_
#define SNAKEBITE_GAME_OBJECT_H_

#include <string>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "snake_collider.h"

namespace acoross {
namespace snakebite {

class GameObjectClone;

class GameObject
{
public:
	GameObject(MovingObjectContainer& container, ColliderBase* collider, Position2D pos, double radius, std::string name = "noname")
		: container_(container), collider_(collider), head_(pos, radius), Name(name)
	{}
	virtual ~GameObject()
	{
		/*container_.DeleteObject(head_);
		for (auto mo : body_list_)
		{
			container_.DeleteObject(mo);
		}*/
	}
	
	GameObjectClone Clone();

	bool IsCollidingTo(std::shared_ptr<GameObject> other) const
	{
		if (this == other.get())
		{
			return false;
		}

		if (IsCrashed(head_, other->head_))
		{
			return true;
		}

		for (auto mo : other->body_list_)
		{
			if (IsCrashed(head_, mo))
			{
				return true;
			}
		}

		return false;
	}

public:
	std::unique_ptr<ColliderBase> collider_;
	MovingObjectContainer& container_;
	MovingObject head_;
	std::list<MovingObject> body_list_;
	const std::string Name;
};

using GameObjectWP = std::weak_ptr<GameObject>;

class GameObjectClone
{
public:
	GameObjectClone(GameObject& lhs)
		: head_(lhs.head_)
		, body_list_(lhs.body_list_)
		, Name(lhs.Name)
	{}

	GameObjectClone(MovingObject head, std::list<MovingObject> body_list, std::string name)
		: head_(head)
		, body_list_(body_list)
		, Name(name)
	{}

	MovingObject head_;
	std::list<MovingObject> body_list_;

	const std::string Name;
};

inline GameObjectClone GameObject::Clone()
{
	return GameObjectClone(*this);
}

}
}

#endif //SNAKEBITE_GAME_OBJECT_H_