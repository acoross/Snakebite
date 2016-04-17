#pragma once

#include <map>
#include <list>
#include <set>

#include "snake.h"
#include "apple.h"
#include "handle.h"

namespace acoross {
namespace snakebite {

using MapSnake = std::map<Handle<Snake>::Type, SnakeSP>;
using ListApple = std::list<AppleSP>;
using CollisionMap = std::map<Handle<Snake>::Type, GameObjectWP>;
using CollisionSet = std::set<Handle<Snake>::Type>;

}
}
