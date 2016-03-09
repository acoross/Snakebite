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
	virtual ~ColliderBase(){}
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

class SnakePiece;

class PlayerHeadCollider : public ColliderBase
{
public:
	PlayerHeadCollider(SnakePiece* owner)
		: owner_(owner)
	{
	}
	virtual ~PlayerHeadCollider()
	{
		return;
	}

	virtual void Collide(ColliderBase& other, int cnt) override
	{
		other.Collide(*this, cnt + 1);
	}

	virtual void Collide(SnakeBodyCollider& other, int cnt) override;

	virtual void Collide(SnakeHeadCollider& other, int cnt) override;

	virtual void Collide(PlayerHeadCollider& other, int cnt) override;

	SnakePiece* owner_;
};

}
}

#endif //SNAKEBITE_SNAKEBITE_COLLIDER_H_