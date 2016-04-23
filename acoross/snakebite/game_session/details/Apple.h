#ifndef SNAKEBITE_APPLE_H_
#define SNAKEBITE_APPLE_H_

#include "sb_zone_object.h"
#include "snake_collider.h"

namespace acoross {
namespace snakebite {

class Apple : public SbZoneObject
{
public:
	Apple(const Position2D& pos, double radius)
		: SbZoneObject(new AppleCollider(this), pos, radius)
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
