#ifndef SNAKEBITE_M1_MOVING_OBJECT_H_
#define SNAKEBITE_M1_MOVING_OBJECT_H_

#include <iostream>
#include <functional>

#include "geo_types.h"

namespace acoross {
namespace snakebite {

// 위치와 반경을 지니는 기본 객체.
class MovingObject final
{
public:
	using MyType = MovingObject;
	using MyContainer = Rect;

	MovingObject(const Position2D& pos, double radius)
		: pos_(pos), radius_(radius)
	{}
	~MovingObject() {}

	void Move(const DirVector2D& diff);
	void Move(const DirVector2D& diff, Rect& boundary);

	void SetPosition(const Position2D& newpos)
	{
		pos_ = newpos;
	}
	Position2D GetPosition() const { return pos_; }
	double GetRadius() const { return radius_; }

private:
	Position2D pos_;
	double radius_;
};

inline void MovingObject::Move(const DirVector2D & diff)
{
	auto pos_new = pos_;
	pos_new.x += diff.x;
	pos_new.y += diff.y;

	pos_ = pos_new;
}

inline void MovingObject::Move(const DirVector2D & diff, Rect& boundary)
{
	auto pos_new = pos_;
	pos_new.x += diff.x;
	pos_new.y += diff.y;

	pos_new.x = Trim<double>(pos_new.x, boundary.Left, boundary.Right - 1);
	pos_new.y = Trim<double>(pos_new.y, boundary.Top, boundary.Bottom - 1);

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