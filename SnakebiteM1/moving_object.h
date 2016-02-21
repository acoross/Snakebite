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

	MovingObject(const Position2D& pos, const Degree& angle, double velocity, double ang_vel, double radius)
		: pos_(pos), angle_(angle), velocity_(velocity), ang_vel_(ang_vel), radius_(radius)
	{}
	virtual ~MovingObject(){}

	void MoveTo(const Position2D& newpos)
	{
		pos_ = newpos;
	}
	
	void Move(const DirVector2D& diff)
	{
		pos_.x += diff.x;
		pos_.y += diff.y;
	}

	void SetAngle(const Degree& angle)
	{
		angle_ = angle;
	}

	void Turn(const Degree& diff)
	{
		angle_ = angle_ + diff;
	}
	
	Position2D GetPosition() const { return pos_; }
	Degree GetAngle() const { return angle_; }
	double GetVelocity() const { return velocity_; }
	double GetAngVelocity() const { return ang_vel_; }
	double GetRadius() const { return radius_; }

private:
	Position2D pos_;	// relational positino to field, as UNIT
	Degree angle_; // degree
	double velocity_; // UNIT/ms
	double ang_vel_;	// degree/ms
	double radius_;
};

}
}

#endif //SNAKEBITE_M1_MOVING_OBJECT_H_