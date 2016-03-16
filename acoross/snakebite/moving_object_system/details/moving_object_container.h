#ifndef SNAKEBITE_MOVING_OBJECT_CONTAINER_H_
#define SNAKEBITE_MOVING_OBJECT_CONTAINER_H_

#include <list>
#include <map>
#include <memory>

#include "geo_types.h"

namespace acoross {
namespace snakebite {

class MovingObject;

class MovingObjectContainer
{
public:
	using MovingObjectSP = std::shared_ptr<MovingObject>;
	using MovingObjectWP = std::weak_ptr<MovingObject>;
	typedef std::list<MovingObjectSP> ListMovingObject;

	// game field APIs
	const int Left{ 0 };
	const int Right{ 500 };
	const int Top{ 0 };
	const int Bottom{ 500 };

	int Width() const { return Right - Left; }
	int Height() const { return Top - Bottom; }

private:	// deprecate
	void RegisterMovingObject(MovingObjectSP mo)
	{
		moving_objects_.push_back(mo);
	}

	void DeleteObject(MovingObjectSP mo)
	{
		for (auto it = moving_objects_.begin(); it != moving_objects_.end(); ++it)
		{
			if (*it == mo)
			{
				moving_objects_.erase(it);
				return;
			}
		}
	}

private:
	// unique_ptr 이라서 자동 삭제됨.
	ListMovingObject moving_objects_;
};

}
}
#endif SNAKEBITE_MOVING_OBJECT_CONTAINER_H_