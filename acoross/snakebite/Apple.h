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
		: GameObject(container)
	{
		auto mo = std::make_shared<MyMovingObject>(container, pos, radius, new DummyCollider(this));
		container.RegisterMovingObject(mo);
		mo_ = mo;
	}

	virtual ~Apple(){}

private:
	std::shared_ptr<MyMovingObject> mo_;
};

}
}
#endif //SNAKEBITE_APPLE_H_
