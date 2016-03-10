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

	void RegisterMovingObject(MovingObjectSP mo)
	{
		moving_objects_.push_back(mo);
	}

	void DeleteObject(MovingObjectSP mo_wp)
	{
		for (auto it = moving_objects_.begin(); it != moving_objects_.end(); ++it)
		{
			moving_objects_.erase(it);
		}
	}

	//임시로 열어주는 API
	ListMovingObject& GetMovingObjects() { return moving_objects_; }

private:
	// unique_ptr 이라서 자동 삭제됨.
	ListMovingObject moving_objects_;
};

//inline void MovingObjectContainer::CheckCollisions()
//{
//	ListMovingObject& mo_list = moving_objects_;
//
//	//// clean up collision
//	//for (auto& mo : mo_list)
//	//{
//	//	mo->Collided = false;
//	//}
//
//	for (auto& mo1 : mo_list)
//	{
//		for (auto& mo2 : mo_list)
//		{
//			if (mo1.get() == mo2.get())
//				continue;
//
//			mo1->Collide(*mo2);
//			mo2->Collide(*mo1);
//
//			/*MovingObject::ProcessCollsion(*mo1, *mo2);*/
//		}
//	}
//}


}
}
#endif SNAKEBITE_MOVING_OBJECT_CONTAINER_H_