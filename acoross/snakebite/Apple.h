#ifndef SNAKEBITE_APPLE_H_
#define SNAKEBITE_APPLE_H_

#include "game_object.h"

namespace acoross {
namespace snakebite {

class Apple : public GameObject
{
public:
	Apple(MyContainer& container
		, const Position2D& pos, double radius)
		: GameObject(container, new AppleCollider(this))
	{
		auto mo = std::make_shared<MyMovingObject>(container, pos, radius);
		container.RegisterMovingObject(mo);
		head_ = mo;
	}

	virtual ~Apple(){}

private:
};

}
}
#endif //SNAKEBITE_APPLE_H_
