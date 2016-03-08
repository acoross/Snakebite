#ifndef SNAKEBITE_M1_MOVING_OBJECT_H_
#define SNAKEBITE_M1_MOVING_OBJECT_H_

#include <iostream>
#include <functional>

#include "geo_types.h"
#include "moving_object_container.h"

namespace acoross {
namespace snakebite {

//reference type
template <typename TCollider>	//TCollider 는 void Collider(TCollider&); 를 구현해야 한다.
class MovingObject
{
public:
	typedef std::function<void(MovingObject& mo)> OnCollideCollback;

	MovingObject(MovingObject&) = delete;
	MovingObject& operator=(MovingObject&) = delete;

	MovingObject(MovingObjectContainer<MovingObject>& container, int Id, const Position2D& pos, double radius, TCollider* collider)
		: container_(container), Id_(Id), pos_(pos), radius_(radius), collider_(collider)
	{}
	virtual ~MovingObject()
	{}

	virtual void MoveTo(const Position2D& newpos)
	{
		pos_ = newpos;
	}
	
	virtual void Move(const DirVector2D& diff);

	Position2D GetPosition() const { return pos_; }
	double GetRadius() const { return radius_; }
		
	//static void ProcessCollsion(MovingObject& m1, MovingObject& m2);

	bool Collided{ false };

	int GetId() const { return Id_; }
	
	void SetCollideCollback(OnCollideCollback collideCallback)
	{
		collideCallback_ = collideCallback;
	}

	void Collide(MovingObject& other)
	{
		collider_->Collide(*other.collider_, 0);
	}

	OnCollideCollback collideCallback_;
	
private:
	MovingObjectContainer<MovingObject>& container_;

	int Id_;
	Position2D pos_;	// relational positino to field, as UNIT
	double radius_;

	std::unique_ptr<TCollider> collider_;
};

template <typename TCollider>
bool IsCrashed(const MovingObject<TCollider>& mo1, const MovingObject<TCollider>& mo2)
{
	double dist = Position2D::Distance(mo1.GetPosition(), mo2.GetPosition());

	if (dist < mo1.GetRadius() + mo2.GetRadius())
	{
		return true;
	}

	return false;
}

template <typename TCollider>
void MovingObject<TCollider>::Move(const DirVector2D & diff)
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

}
}
#endif //SNAKEBITE_M1_MOVING_OBJECT_H_