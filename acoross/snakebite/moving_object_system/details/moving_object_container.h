#ifndef SNAKEBITE_MOVING_OBJECT_CONTAINER_H_
#define SNAKEBITE_MOVING_OBJECT_CONTAINER_H_

#include <list>
#include <map>
#include <memory>

#include "geo_types.h"

namespace acoross {
namespace snakebite {

class MovingObject;

class MovingObjectContainer final
{
public:
	MovingObjectContainer(int left, int right, int top, int bottom)
		: Left(left), Right(right), Top(top), Bottom(bottom)
	{}

	// game field APIs
	const int Left{ 0 };
	const int Right{ 500 };
	const int Top{ 0 };
	const int Bottom{ 500 };

	int Width() const { return Right - Left; }
	int Height() const { return Bottom - Top; }
};

}
}
#endif SNAKEBITE_MOVING_OBJECT_CONTAINER_H_