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
	GameObject(ColliderBase* collider, Position2D pos, double radius, std::string name = "noname")
		: collider_(collider), head_(pos, radius), Name(name), zone_idx_x_(0), zone_idx_y_(0)
	{}
	virtual ~GameObject()
	{}
	
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

	std::pair<int, int> AtomicZoneIdx() const
	{
		return std::make_pair(zone_idx_x_.load(), zone_idx_y_.load());
	}

	std::pair<int, int> AtomicZoneIdx(int idx_x, int idx_y)
	{
		return std::make_pair(zone_idx_x_.exchange(idx_x), zone_idx_y_.exchange(idx_y));
	}

public:
	std::atomic<int> zone_idx_x_;
	std::atomic<int> zone_idx_y_;
	std::unique_ptr<ColliderBase> collider_;
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
		, zone_idx_x_(lhs.zone_idx_x_.load())
		, zone_idx_y_(lhs.zone_idx_y_.load())
	{}

	GameObjectClone(MovingObject head, std::list<MovingObject> body_list, std::string name, int zone_idx_x, int zone_idx_y)
		: head_(head)
		, body_list_(body_list)
		, Name(name)
		, zone_idx_x_(zone_idx_x)
		, zone_idx_y_(zone_idx_y)
	{}

	MovingObject head_;
	std::list<MovingObject> body_list_;

	const std::string Name;

	int zone_idx_x_;
	int zone_idx_y_;
};

inline GameObjectClone GameObject::Clone()
{
	return GameObjectClone(*this);
}

}
}

#endif //SNAKEBITE_GAME_OBJECT_H_