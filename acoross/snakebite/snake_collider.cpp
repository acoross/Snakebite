#include "snake_collider.h"

#include "snake.h"

namespace acoross {
namespace snakebite {

//ColliderImpl(SnakeHeadCollider)
void SnakeHeadCollider::Collide(SnakeBodyCollider & other, int cnt)
{
	//if (owner_ == other.owner_)
	//	return;

	//auto it = collided_set_.insert(other.owner_);
	//if (it.second)
	//{
	//	// on collide
	//	owner_->Turn(60);
	//}

	return;
}

void SnakeHeadCollider::Collide(SnakeHeadCollider & other, int cnt)
{
	return;
}

void SnakeHeadCollider::Collide(PlayerHeadCollider & other, int cnt)
{
	return;
}

//ColliderImpl(SnakeBodyCollider)
void SnakeBodyCollider::Collide(SnakeBodyCollider & other, int cnt)
{
	return;
}

void SnakeBodyCollider::Collide(SnakeHeadCollider & other, int cnt)
{
	return;
}

void SnakeBodyCollider::Collide(PlayerHeadCollider & other, int cnt)
{
	return;
}

//ColliderImpl(PlayerHeadCollider)
void PlayerHeadCollider::Collide(SnakeBodyCollider & other, int cnt)
{
	//if (owner_ == other.owner_)
	//	return;

	//auto it = collided_set_.insert(other.owner_);
	//if (it.second)
	//{
	//	// on collide
	//	owner_->Turn(60);
	//}

	return;
}

void PlayerHeadCollider::Collide(SnakeHeadCollider & other, int cnt)
{
	return;
}

void PlayerHeadCollider::Collide(PlayerHeadCollider & other, int cnt)
{
	return;
}

//ColliderImpl(DummyCollider)
void DummyCollider::Collide(SnakeBodyCollider & other, int cnt)
{
	return;
}

void DummyCollider::Collide(SnakeHeadCollider & other, int cnt)
{
	return;
}

void DummyCollider::Collide(PlayerHeadCollider & other, int cnt)
{
	return;
}

}
}
