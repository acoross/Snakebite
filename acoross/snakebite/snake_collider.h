#ifndef SNAKEBITE_SNAKEBITE_COLLIDER_H_
#define SNAKEBITE_SNAKEBITE_COLLIDER_H_

#include <set>
#include <acoross/snakebite/moving_object_system/moving_object_system.h>

namespace acoross {
namespace snakebite {

class GameObject;

class SnakeHeadCollider;
class SnakeBodyCollider;
class PlayerHeadCollider;
class DummyCollider;

class ColliderBase
{
public:
	ColliderBase() = delete;
	ColliderBase(GameObject* owner)
		: owner_(owner)
	{}
	virtual ~ColliderBase(){}

	virtual void Collide(ColliderBase& other, int cnt) = 0;
	virtual void Collide(SnakeHeadCollider& other, int cnt) = 0;
	virtual void Collide(SnakeBodyCollider& other, int cnt) = 0;
	virtual void Collide(PlayerHeadCollider& other, int cnt) = 0;
	virtual void Collide(DummyCollider& other, int cnt)
	{
		return;
	}

	void ReleaseCollision(ColliderBase& other)
	{
	}

	GameObject* owner_;
};

#define ColliderImpl(T) \
class T : public ColliderBase \
{	\
public:	\
	T(GameObject* owner) : ColliderBase(owner){}	\
	virtual void Collide(ColliderBase& other, int cnt) override	\
	{	\
		other.Collide(*this, cnt + 1);	\
	}	\
	virtual void Collide(SnakeHeadCollider& other, int cnt) override;	\
	virtual void Collide(SnakeBodyCollider& other, int cnt) override;	\
	virtual void Collide(PlayerHeadCollider& other, int cnt) override;	\
};

ColliderImpl(SnakeHeadCollider)
ColliderImpl(SnakeBodyCollider)
ColliderImpl(PlayerHeadCollider)
ColliderImpl(DummyCollider)
#undef ColliderImpl

}
}

#endif //SNAKEBITE_SNAKEBITE_COLLIDER_H_