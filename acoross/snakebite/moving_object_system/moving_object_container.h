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
	typedef std::list<std::shared_ptr<MovingObject>> ListMovingObject;

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

	void DeleteObject(MovingObject* mo)
	{
		for (auto it = moving_objects_.begin(); it != moving_objects_.end(); ++it)
		{
			if (it->get() == mo)
			{
				moving_objects_.erase(it);
				return;
			}
		}
	}

	void CheckCollisions();

	//임시로 열어주는 API
	ListMovingObject& GetMovingObjects() { return moving_objects_; }

private:
	// unique_ptr 이라서 자동 삭제됨.
	ListMovingObject moving_objects_;
};

}
}
#endif SNAKEBITE_MOVING_OBJECT_CONTAINER_H_