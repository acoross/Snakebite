#ifndef SNAKEBITE_COLLIDER_CONTAINER_H_
#define SNAKEBITE_COLLIDER_CONTAINER_H_

#include <map>
#include <memory>

namespace acoross {
namespace snakebite {

class Collider;
class ColliderHandle;

class ColliderContainer
{
public:
	typedef std::map<Collider*, std::weak_ptr<ColliderHandle>> MapColliderHandle;

	//임시로 열어주는 API
	MapColliderHandle& GetColliderHandles()
	{
		return collider_handles_;
	}

	void RegisterCollider(Collider* key, std::shared_ptr<ColliderHandle>& collider_h)
	{
		collider_handles_[key] = collider_h;
	}

	void UnregisterCollider(Collider* key)
	{
		auto it = collider_handles_.find(key);
		if (it != collider_handles_.end())
			collider_handles_.erase(it);
	}

	void CheckCollisions();

	void ProcessCollsion(ColliderHandle& c1, ColliderHandle& c2);
	void ProcessCollsion(Collider& c1, Collider& c2);

private:
	MapColliderHandle collider_handles_;
	MapColliderHandle map_collision_found_;
};

}
}
#endif //SNAKEBITE_COLLIDER_CONTAINER_H_