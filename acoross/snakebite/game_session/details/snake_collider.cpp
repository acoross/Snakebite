#include "snake_collider.h"

#include "snake.h"
#include "Apple.h"
#include "game_session.h"
#include "sb_zone_object.h"

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
	owner_->Die();
	return;
}
void SnakeCollider::Collide(SnakeTailCollider& other, int cnt)
{
	if (Handle<Snake>(owner_).handle != other.owner_->owner_handle_)
	{
		owner_->Die();
	}
	
	return;
}
void SnakeCollider::Collide(AppleCollider& other, int cnt)
{
	auto handle = Handle<Snake>(owner_).handle;
	owner_->game_session_.RequestRemoveApple(
		Handle<SbZoneObject>(other.owner_).handle,
		[handle, &gs = owner_->game_session_](bool ret)
	{
		if (ret)
		{
			gs.RequestMakeNewApple();
			gs.RequestToSnake(
				handle,
				[](Snake& owner)
			{
				owner.AddBody();
			});
		}
	});

	return;
}

//ColliderImpl(SnakeTailCollider)
void SnakeTailCollider::Collide(SnakeCollider& other, int cnt)
{
	return;
}
void SnakeTailCollider::Collide(SnakeTailCollider& other, int cnt)
{
	return;
}
void SnakeTailCollider::Collide(AppleCollider& other, int cnt)
{
	return;
}

//ColliderImpl(AppleCollider)
void AppleCollider::Collide(SnakeCollider& other, int cnt)
{
	return;
}
void AppleCollider::Collide(SnakeTailCollider& other, int cnt)
{
	return;
}
void AppleCollider::Collide(AppleCollider& other, int cnt)
{
	return;
}

}
}
