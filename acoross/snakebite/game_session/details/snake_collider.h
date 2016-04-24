#ifndef SNAKEBITE_SNAKEBITE_COLLIDER_H_
#define SNAKEBITE_SNAKEBITE_COLLIDER_H_

#include <set>
#include <acoross/snakebite/moving_object_system/moving_object_system.h>

#include "collider_base.h"

namespace acoross {
namespace snakebite {

class Snake;
class SnakeTail;
class Apple;

template <typename T>
class ZoneObjectEx;
using SbZoneObject = ZoneObjectEx<SbColliderBase>;

ColliderImpl(SnakeCollider, Snake)
ColliderImpl(SnakeTailCollider, SnakeTail)
ColliderImpl(AppleCollider, Apple)
ColliderImpl(DummyCollider, SbZoneObject)

#undef ColliderImpl

}
}

#endif //SNAKEBITE_SNAKEBITE_COLLIDER_H_