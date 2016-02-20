#ifndef SNAKEBITE_M1_MOVING_OBJECT_H_
#define SNAKEBITE_M1_MOVING_OBJECT_H_

#include "geo_types.h"

namespace acoross {
namespace snakebite {

class MovingObject
{
public:
	MovingObject(MovingObject&) = delete;
	MovingObject& operator=(MovingObject&) = delete;

	MovingObject(Position2D pos, Degree angle)
		: pos_(pos), angle_(angle)
	{}

	void MoveTo(Position2D newpos)
	{
		pos_ = newpos;
	}
	
	void Move(DirVector2D diff)
	{
		pos_.x += diff.x;
		pos_.y += diff.y;
	}

	void SetAngle(Degree angle)
	{
		angle_ = angle;
	}

	void Turn(Degree diff)
	{
		angle_ = angle_ + diff;
	}

private:
	Position2D pos_;
	Degree angle_;
};

}
}

#endif //SNAKEBITE_M1_MOVING_OBJECT_H_