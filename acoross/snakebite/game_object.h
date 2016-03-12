#ifndef SNAKEBITE_GAME_OBJECT_H_
#define SNAKEBITE_GAME_OBJECT_H_

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include <acoross/util.h>
#include "snake_collider.h"

namespace acoross {
namespace snakebite {

class GameObject
{
public:
	using MyMovingObject = MovingObject;
	using MyContainer = MovingObjectContainer;
	
	GameObject(MyContainer& container, ColliderBase* collider)
		: container_(container), collider_(collider)
	{}
	virtual ~GameObject()
	{
		container_.DeleteObject(head_);
		for (auto mo : body_list_)
		{
			container_.DeleteObject(mo);
		}
	}
	
	bool IsCollidingTo(std::shared_ptr<GameObject> other) const
	{
		if (this == other.get())
		{
			return false;
		}

		if (IsCrashed(*head_, *other->head_))
		{
			return true;
		}

		for (auto mo : other->body_list_)
		{
			if (IsCrashed(*head_, *mo))
			{
				return true;
			}
		}

		return false;
	}

public:
	std::unique_ptr<ColliderBase> collider_;
	MyContainer& container_;
	std::shared_ptr<MyMovingObject> head_;
	std::list<std::shared_ptr<MyMovingObject>> body_list_;
};

}
}

#endif //SNAKEBITE_GAME_OBJECT_H_