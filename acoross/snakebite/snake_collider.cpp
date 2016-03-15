#include "snake_collider.h"

#include "snake.h"
#include "Apple.h"
#include "game_session.h"

namespace acoross {
namespace snakebite {

//ColliderImpl(DummyCollider)
void DummyCollider::Collide(SnakeCollider& other, int cnt)
{
	return;
}
void DummyCollider::Collide(AppleCollider& other, int cnt)
{
	return;
}

//ColliderImpl(SnakeCollider)
void SnakeCollider::Collide(SnakeCollider& other, int cnt)
{
	if (owner_->game_session_.RemoveSnake(owner_))
	{
		owner_->game_session_.AddSnake();
	}

	return;
}
void SnakeCollider::Collide(AppleCollider& other, int cnt)
{
	if (owner_->game_session_.RemoveApple(other.owner_))
	{
		owner_->game_session_.AddApple();
		owner_->AddBody();
	}

	return;
}

//ColliderImpl(AppleCollider)
void AppleCollider::Collide(SnakeCollider& other, int cnt)
{
	return;
}
void AppleCollider::Collide(AppleCollider& other, int cnt)
{
	return;
}

}
}
