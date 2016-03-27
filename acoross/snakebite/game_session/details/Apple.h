#ifndef SNAKEBITE_APPLE_H_
#define SNAKEBITE_APPLE_H_

#include "game_object.h"

namespace acoross {
namespace snakebite {

class Apple : public GameObject
{
public:
	Apple(MovingObjectContainer& container
		, const Position2D& pos, double radius)
		: GameObject(container, new AppleCollider(this), pos, radius)
	{
		//container.RegisterMovingObject(mo);
	}
	Apple(Apple& lhs) = default;
	Apple(Apple&& lhs) = default;

	virtual ~Apple(){}

private:
};
using AppleSP = std::shared_ptr<Apple>;

}
}
#endif //SNAKEBITE_APPLE_H_