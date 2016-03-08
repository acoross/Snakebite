#ifndef SNAKEBITE_MOVING_OBJECT_CONTAINER_H_
#define SNAKEBITE_MOVING_OBJECT_CONTAINER_H_

#include <list>
#include <map>
#include <memory>

#include "geo_types.h"

namespace acoross {
namespace snakebite {

template <typename TMovingObject>
class MovingObjectContainer
{
public:
	using MovingObjectSP = std::shared_ptr<TMovingObject>;
	using MovingObjectWP = std::weak_ptr<TMovingObject>;
	typedef std::list<MovingObjectSP> ListMovingObject;

	// game field APIs
	const int Left{ 0 };
	const int Right{ 500 };
	const int Top{ 0 };
	const int Bottom{ 500 };

	int Width() const { return Right - Left; }
	int Height() const { return Top - Bottom; }

	template<typename T, typename... Args>
	std::weak_ptr<T> CreateMovingObject(Args&&... args)
	{
		auto mo_new = std::make_shared<T>(*this, std::forward<Args>(args)...);
		moving_objects_.push_back(mo_new);

		return mo_new;
	}

	void DeleteObject(MovingObjectWP mo_wp)
	{
		if (auto mo = mo_wp.lock())
		{
			DeleteObject(mo.get());
		}
	}

	void DeleteObject(TMovingObject* mo)
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

template <typename TMovingObject>
void MovingObjectContainer<TMovingObject>::CheckCollisions()
{
	ListMovingObject& mo_list = moving_objects_;

	// clean up collision
	for (auto& mo : mo_list)
	{
		mo->Collided = false;
	}

	for (auto& mo1 : mo_list)
	{
		for (auto& mo2 : mo_list)
		{
			if (mo1.get() == mo2.get())
				continue;

			mo1->Collide(*mo2);
			mo2->Collide(*mo1);

			/*MovingObject::ProcessCollsion(*mo1, *mo2);*/
		}
	}
}

}
}
#endif SNAKEBITE_MOVING_OBJECT_CONTAINER_H_