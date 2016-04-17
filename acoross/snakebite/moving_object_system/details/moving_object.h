#ifndef SNAKEBITE_M1_MOVING_OBJECT_H_
#define SNAKEBITE_M1_MOVING_OBJECT_H_

#include <iostream>
#include <functional>

#include "geo_types.h"
#include "moving_object_container.h"

namespace acoross {
namespace snakebite {

//template <typename TCollider>	//TCollider 는 void Collider(TCollider&); 를 구현해야 한다.
class MovingObject
{
public:
	using MyType = MovingObject;//<TCollider>;
	using MyContainer = MovingObjectContainer;//<MyType>;

	MovingObject(const Position2D& pos, double radius)
		: pos_(pos), radius_(radius)
	{}
	virtual ~MovingObject()	{}

	virtual void MoveTo(const Position2D& newpos)
	{
		pos_ = newpos;
	}
	
	void Move(const DirVector2D& diff, MyContainer& container);

	Position2D GetPosition() const { return pos_; }
	double GetRadius() const { return radius_; }
	
private:
	Position2D pos_;	// relational positino to field, as UNIT
	double radius_;
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

inline void MovingObject::Move(const DirVector2D & diff, MovingObjectContainer& container)
{
	// 테두리 밖으로 벗어나지 않도록 막음.
	auto pos_new = pos_;
	pos_new.x += diff.x;
	pos_new.y += diff.y;
	
	pos_new.x = Trim<double>(pos_new.x, container.Left, container.Right - 1);
	pos_new.y = Trim<double>(pos_new.y, container.Top, container.Bottom - 1);

	pos_ = pos_new;
}

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