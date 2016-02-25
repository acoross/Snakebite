#ifndef SNAKEBITE_M1_MOVING_OBJECT_H_
#define SNAKEBITE_M1_MOVING_OBJECT_H_

#include <iostream>

#include "geo_types.h"
#include "moving_object_container.h"

namespace acoross {
namespace snakebite {

class MovingObject
{
public:
	MovingObject(MovingObject&) = delete;
	MovingObject& operator=(MovingObject&) = delete;

	MovingObject(MovingObjectContainer& container, int Id, const Position2D& pos, double radius)
		: container_(container), Id_(Id), pos_(pos), radius_(radius)
	{}
	virtual ~MovingObject(){}

	void MoveTo(const Position2D& newpos)
	{
		pos_ = newpos;
	}
	
	void Move(const DirVector2D& diff);

	Position2D GetPosition() const { return pos_; }
	double GetRadius() const { return radius_; }
		
	static void ProcessCollsion(MovingObject& m1, MovingObject& m2);

	bool Collided{ false };

	int GetId() const { return Id_; }

private:
	MovingObjectContainer& container_;

	int Id_;
	Position2D pos_;	// relational positino to field, as UNIT
	double radius_;
};

bool IsCrashed(const MovingObject& mo1, const MovingObject& mo2);

}
}
#endif //SNAKEBITE_M1_MOVING_OBJECT_H_