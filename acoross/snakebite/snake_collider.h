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

template <typename TCollider>
class Foo
{
public:
	Foo(TCollider* c)
		: c_(c)
	{}

	void Collide(Foo& other)
	{
		other.c_->Collide(*c_, 0);
	}

	std::unique_ptr<TCollider> c_;
};

class SnakeHeadCollider : public ColliderBase
{
public:
	virtual void Collide(ColliderBase& other, int cnt)
	{
		other.Collide(*this, cnt + 1);
	}

	virtual void Collide(SnakeHeadCollider& other, int cnt)
	{
		return;
	}

	virtual void Collide(SnakeBodyCollider& other, int cnt)
	{
		return;
	}

	virtual void Collide(PlayerHeadCollider& other, int cnt)
	{
		return;
	}
};

class SnakeBodyCollider : public ColliderBase
{
public:
	virtual void Collide(ColliderBase& other, int cnt)
	{
		other.Collide(*this, cnt + 1);
	}

	virtual void Collide(SnakeBodyCollider& other, int cnt)
	{
		return;
	}

	virtual void Collide(SnakeHeadCollider& other, int cnt)
	{
		return;
	}

	virtual void Collide(PlayerHeadCollider& other, int cnt)
	{
		return;
	}
};

class PlayerHeadCollider : public ColliderBase
{
public:
	virtual void Collide(ColliderBase& other, int cnt)
	{
		other.Collide(*this, cnt + 1);
	}

	virtual void Collide(SnakeBodyCollider& other, int cnt)
	{
		return;
	}

	virtual void Collide(SnakeHeadCollider& other, int cnt)
	{
		return;
	}

	virtual void Collide(PlayerHeadCollider& other, int cnt)
	{
		return;
	}
};

}
}

#endif //SNAKEBITE_SNAKEBITE_COLLIDER_H_