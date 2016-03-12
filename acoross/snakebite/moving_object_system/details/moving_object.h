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
	using MyType = MovingObject<TCollider>;
	using MyContainer = MovingObjectContainer<MyType>;

	MovingObject(MovingObject&) = delete;
	MovingObject& operator=(MovingObject&) = delete;

	MovingObject(MyContainer& container, const Position2D& pos, double radius, TCollider* collider)
		: container_(container), pos_(pos), radius_(radius), collider_(collider)
	{}
	virtual ~MovingObject()	{}

	virtual void MoveTo(const Position2D& newpos)
	{
		pos_ = newpos;
	}
	
	virtual void Move(const DirVector2D& diff);

	Position2D GetPosition() const { return pos_; }
	double GetRadius() const { return radius_; }
	
	void Collide(MovingObject& other)
	{
		collider_->Collide(*other.collider_, 0);
	}

	void ReleaseCollision(MovingObject& other)
	{
		collider_->ReleaseCollision(*other.collider_);
	}
	
private:
	MyContainer& container_;

	Position2D pos_;	// relational positino to field, as UNIT
	double radius_;

	std::unique_ptr<TCollider> collider_;
};

template <typename T>
T Trim(T src, T minv, T maxv)
{
	_ASSERT(minv <= maxv);

	if (src < minv)
		return minv;
	if (src > maxv)
		return maxv;
	return src;
}

template <typename TCollider>
inline void MovingObject<TCollider>::Move(const DirVector2D & diff)
{
	// 테두리 밖으로 벗어나지 않도록 막음.
	auto pos_new = pos_;
	pos_new.x += diff.x;
	pos_new.y += diff.y;
	
	pos_new.x = Trim<double>(pos_new.x, container_.Left, container_.Right);
	pos_new.y = Trim<double>(pos_new.y, container_.Top, container_.Bottom);

	pos_ = pos_new;
}

template <typename TCollider>
inline bool IsCrashed(const MovingObject<TCollider>& mo1, const MovingObject<TCollider>& mo2)
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