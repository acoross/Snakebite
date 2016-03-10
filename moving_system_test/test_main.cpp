#include <iostream>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>

int main()
{
	using namespace acoross::snakebite;

	MovingObjectContainer container_;
	auto mo = std::make_shared<MovingObject>(container_, 1, Position2D(0., 0.), 1.0);
	container_.RegisterMovingObject(mo);
	
	return 0;
}