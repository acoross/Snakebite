#ifndef SNAKEBITE_SNAKEBITE_COLLIDER_H_
#define SNAKEBITE_SNAKEBITE_COLLIDER_H_

#include <set>
#include <acoross/snakebite/moving_object_system/moving_object_system.h>

namespace acoross {
namespace snakebite {

class Snake;

class SnakeHeadCollider;
class SnakeBodyCollider;
class PlayerHeadCollider;

class ColliderBase
{
public:
	ColliderBase() = delete;
	ColliderBase(Snake* owner)
		: owner_(owner)
	{}
	virtual ~ColliderBase(){}

	virtual void Collide(ColliderBase& other, int cnt) = 0;
	virtual void Collide(SnakeHeadCollider& other, int cnt) = 0;
	virtual void Collide(SnakeBodyCollider& other, int cnt) = 0;
	virtual void Collide(PlayerHeadCollider& other, int cnt) = 0;
	
	void ReleaseCollision(ColliderBase& other)
	{
		collided_set_.erase(other.owner_);
	}

	Snake* owner_;

	std::set<Snake*> collided_set_;
};

#define ColliderImpl(T) \
class T : public ColliderBase \
{	\
public:	\
	T(Snake* owner) : ColliderBase(owner){}	\
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
#undef ColliderImpl

}
}

#endif //SNAKEBITE_SNAKEBITE_COLLIDER_H_