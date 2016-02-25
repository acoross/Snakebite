#include "moving_object_container.h"
#include "moving_object.h"

namespace acoross {
namespace snakebite {

void MovingObjectContainer::CheckCollisions()
{
	// clean up collision
	for (auto& mo : moving_objects_)
	{
		mo->Collided = false;
	}

	for (auto& mo1 : moving_objects_)
	{
		for (auto& mo2 : moving_objects_)
		{
			if (mo1.get() == mo2.get())
				continue;

			MovingObject::ProcessCollsion(*mo1, *mo2);
		}
	}
}

}
}