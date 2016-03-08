#ifndef SNAKEBITE_SNAKEBITE_COLLIDER_H_
#define SNAKEBITE_SNAKEBITE_COLLIDER_H_

#include <acoross/snakebite/moving_object_system/moving_object_system.h>

namespace acoross {
namespace snakebite {

class SnakeHeadCollider;
class SnakeBodyCollider;
class PlayerHeadCollider;

class ColliderBase
{
public:
	virtual void Collide(ColliderBase& other, int cnt) = 0;
	virtual void Collide(SnakeHeadCollider& other, int cnt) = 0;
	virtual void Collide(SnakeBodyCollider& other, int cnt) = 0;
	virtual void Collide(PlayerHeadCollider& other, int cnt) = 0;
};

class SnakeHeadCollider : public ColliderBase
{
public:
	virtual void Collide(ColliderBase& other, int cnt) override
	{
		other.Collide(*this, cnt + 1);
	}

	virtual void Collide(SnakeHeadCollider& other, int cnt) override
	{
		return;
	}

	virtual void Collide(SnakeBodyCollider& other, int cnt) override
	{
		return;
	}

	virtual void Collide(PlayerHeadCollider& other, int cnt) override
	{
		return;
	}
};

class SnakeBodyCollider : public ColliderBase
{
public:
	virtual void Collide(ColliderBase& other, int cnt) override
	{
		other.Collide(*this, cnt + 1);
	}

	virtual void Collide(SnakeBodyCollider& other, int cnt) override
	{
		return;
	}

	virtual void Collide(SnakeHeadCollider& other, int cnt) override
	{
		return;
	}

	virtual void Collide(PlayerHeadCollider& other, int cnt) override
	{
		return;
	}
};

class PlayerHeadCollider : public ColliderBase
{
public:
	virtual void Collide(ColliderBase& other, int cnt) override
	{
		other.Collide(*this, cnt + 1);
	}

	virtual void Collide(SnakeBodyCollider& other, int cnt) override
	{
		return;
	}

	virtual void Collide(SnakeHeadCollider& other, int cnt) override
	{
		return;
	}

	virtual void Collide(PlayerHeadCollider& other, int cnt) override
	{
		return;
	}
};

}
}

#endif //SNAKEBITE_SNAKEBITE_COLLIDER_H_