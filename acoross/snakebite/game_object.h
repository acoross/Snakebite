#ifndef SNAKEBITE_GAME_OBJECT_H_
#define SNAKEBITE_GAME_OBJECT_H_

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include <acoross/util.h>
#include "snake_collider.h"

namespace acoross {
namespace snakebite {

class GameObject
{
public:
	using MyMovingObject = MovingObject<ColliderBase>;
	using MyContainer = MovingObjectContainer<MyMovingObject>;

	GameObject(MyContainer& container)
		: container_(container)
	{}
	virtual ~GameObject(){}
	
protected:
	MyContainer& container_;
};

}
}

#endif //SNAKEBITE_GAME_OBJECT_H_