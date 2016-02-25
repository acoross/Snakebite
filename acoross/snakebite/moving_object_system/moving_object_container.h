#ifndef SNAKEBITE_MOVING_OBJECT_CONTAINER_H_
#define SNAKEBITE_MOVING_OBJECT_CONTAINER_H_

#include <list>
#include <map>
#include <memory>

#include "geo_types.h"
#include "collider_container.h"

namespace acoross {
namespace snakebite {

class MovingObject;

class MovingObjectContainer
{
public:
	typedef std::list<std::unique_ptr<MovingObject>> ListMovingObject;

	// game field APIs
	const int Left{ 0 };
	const int Right{ 500 };
	const int Top{ 0 };
	const int Bottom{ 500 };

	int Width() const { return Right - Left; }
	int Height() const { return Top - Bottom; }

	template<typename... Args>
	MovingObject& CreateMovingObject(Args&&... args)
	{
		MovingObject* mo_new = new MovingObject(*this, std::forward<Args>(args)...);
		moving_objects_.emplace_back(mo_new);

		return *mo_new;
	}

	//임시로 열어주는 API
	ListMovingObject& GetMovingObjects() { return moving_objects_; }

	//임시로 열어주는 API
	//ColliderContainer& GetCollisionContainer() { return collision_container_; }

private:
	// unique_ptr 이라서 자동 삭제됨.
	ListMovingObject moving_objects_;
	//ColliderContainer collision_container_;
};

}
}
#endif SNAKEBITE_MOVING_OBJECT_CONTAINER_H_