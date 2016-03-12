#ifndef SNAKEBITE_SNAKEBITE_COLLIDER_H_
#define SNAKEBITE_SNAKEBITE_COLLIDER_H_

#include <set>
#include <acoross/snakebite/moving_object_system/moving_object_system.h>

namespace acoross {
namespace snakebite {

class GameObject;
class Snake;
class Apple;

class SnakeCollider;
class AppleCollider;
class DummyCollider;

class ColliderBase
{
public:
	virtual ~ColliderBase(){}

	virtual void Collide(ColliderBase& other, int cnt) = 0;
	virtual void Collide(SnakeCollider& other, int cnt) {}
	virtual void Collide(AppleCollider& other, int cnt) {}
	virtual void Collide(DummyCollider& other, int cnt) {}
};

#define ColliderImpl(T, OwnerT) \
class T : public ColliderBase \
{	\
public:	\
	T(OwnerT* owner) : owner_(owner){}	\
	virtual void Collide(ColliderBase& other, int cnt) override	\
	{	\
		other.Collide(*this, cnt + 1);	\
	}	\
	virtual void Collide(SnakeCollider& other, int cnt) override;	\
	virtual void Collide(AppleCollider& other, int cnt) override;	\
\
	OwnerT* owner_;	\
};

ColliderImpl(SnakeCollider, Snake)
ColliderImpl(AppleCollider, Apple)
ColliderImpl(DummyCollider, GameObject)

#undef ColliderImpl

}
}

#endif //SNAKEBITE_SNAKEBITE_COLLIDER_H_