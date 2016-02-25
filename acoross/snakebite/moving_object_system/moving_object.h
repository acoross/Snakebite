#ifndef SNAKEBITE_M1_MOVING_OBJECT_H_
#define SNAKEBITE_M1_MOVING_OBJECT_H_

#include <iostream>

#include "geo_types.h"
#include "moving_object_container.h"
#include "collider.h"

namespace acoross {
namespace snakebite {

class MovingObject
{
public:
	MovingObject(MovingObject&) = delete;
	MovingObject& operator=(MovingObject&) = delete;

	MovingObject(MovingObjectContainer& container, int Id, const Position2D& pos, double radius)
		: container_(container), pos_(pos), radius_(radius), Id_(Id)
	{}
	virtual ~MovingObject(){}

	void MoveTo(const Position2D& newpos)
	{
		pos_ = newpos;
	}
	
	void Move(const DirVector2D& diff);

	Position2D GetPosition() const { return pos_; }
	double GetRadius() const { return radius_; }

	/*void AddCollider()
	{
		collider_.reset(new Collider(container_.GetCollisionContainer(), Id_, *this));
	}

	std::shared_ptr<ColliderHandle> GetColliderHandle() 
	{ 
		if (collider_)
			return collider_->GetHandle();
		else
			return nullptr;
	}
	*/

private:
	MovingObjectContainer& container_;

	Position2D pos_;	// relational positino to field, as UNIT
	double radius_;

	//std::unique_ptr<Collider> collider_;
	int Id_;
};

bool IsCrashed(const MovingObject& mo1, const MovingObject& mo2);

}
}

#endif //SNAKEBITE_M1_MOVING_OBJECT_H_