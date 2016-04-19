#pragma once

#include <unordered_map>
#include <list>
#include <set>

#include "snake.h"
#include "apple.h"
#include "handle.h"

namespace acoross {
namespace snakebite {

using MapSnake = std::unordered_map<Handle<Snake>::Type, SnakeSP>;
using ListApple = std::list<AppleSP>;
using CollisionMap = std::unordered_map<Handle<Snake>::Type, GameObjectWP>;
using CollisionSet = std::set<Handle<Snake>::Type>;

}
}
