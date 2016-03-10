#ifndef SNAKEBITE_M1_MOVING_OBJECT_H_
#define SNAKEBITE_M1_MOVING_OBJECT_H_

#include <iostream>
#include <functional>

#include "geo_types.h"
#include "moving_object_container.h"

namespace acoross {
namespace snakebite {

//reference type
class MovingObject
{
public:
	MovingObject(MovingObject&) = delete;
	MovingObject& operator=(MovingObject&) = delete;

	MovingObject(MovingObjectContainer& container, int Id, const Position2D& pos, double radius)
		: container_(container), pos_(pos), radius_(radius)
	{}
	virtual ~MovingObject()	{}

	virtual void MoveTo(const Position2D& newpos)
	{
		pos_ = newpos;
	}
	
	virtual void Move(const DirVector2D& diff);

	Position2D GetPosition() const { return pos_; }
	double GetRadius() const { return radius_; }
	
private:
	MovingObjectContainer& container_;

	Position2D pos_;	// relational positino to field, as UNIT
	double radius_;
};

inline void MovingObject::Move(const DirVector2D & diff)
{
	// 테두리 밖으로 벗어나지 않도록 막음.
	auto pos_new = pos_;
	pos_new.x += diff.x;
	pos_new.y += diff.y;

	if (pos_new.x > container_.Left && pos_new.x < container_.Right)
	{
		pos_.x = pos_new.x;
	}

	if (pos_new.y > container_.Top && pos_new.y < container_.Bottom)
	{
		pos_.y = pos_new.y;
	}
}

//template <typename TCollider>	//TCollider 는 void Collider(TCollider&); 를 구현해야 한다.
//class MovingObjectWithCollider : public MovingObject
//{
//public:
//	MovingObjectWithCollider(MovingObjectContainer& container, int Id, const Position2D& pos, double radius, TCollider* collider)
//		: MovingObject(container, Id, pos, radius), collider_(collider)
//	{}
//
//	virtual ~MovingObjectWithCollider() {}
//
//	void Collide(MovingObjectWithCollider& other)
//	{
//		collider_->Collide(*other.collider_, 0);
//	}
//
//private:
//	std::unique_ptr<TCollider> collider_;
//};

inline bool IsCrashed(const MovingObject& mo1, const MovingObject& mo2)
{
	double dist = Position2D::Distance(mo1.GetPosition(), mo2.GetPosition());

	if (dist < mo1.GetRadius() + mo2.GetRadius())
	{
		return true;
	}

	return false;
}

}
}
#endif //SNAKEBITE_M1_MOVING_OBJECT_H_