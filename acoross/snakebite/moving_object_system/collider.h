#ifndef SNAKEBITE_COLLIDER_H_
#define SNAKEBITE_COLLIDER_H_

#include <memory>

#include "geo_types.h"
#include "moving_object_container.h"

namespace acoross {
namespace snakebite {

class ColliderHandle
{
public:
	ColliderHandle(Collider& collider)
		: collider_(collider)
	{}

	Collider& GetCollider() { return collider_; }

private:
	Collider& collider_;
};

class Collider
{
public:
	Collider(ColliderContainer& collision_container, int Id, MovingObject& parent)
		: collision_container_(collision_container), parent_(parent), handle_(std::make_shared<ColliderHandle>(*this)), Id_(Id)
	{
		collision_container_.RegisterCollider(this, handle_);
	}
	~Collider()
	{
		collision_container_.UnregisterCollider(this);
	}

	MovingObject& GetMovingObject() { return parent_; }
	std::shared_ptr<ColliderHandle> GetHandle() { return handle_; }

	bool Collided{ false };
	int GetId() const { return Id_; }

private:
	ColliderContainer& collision_container_;
	MovingObject& parent_;
	std::shared_ptr<ColliderHandle> handle_;
	int Id_;
};

}
}

#endif //SNAKEBITE_COLLIDER_H_