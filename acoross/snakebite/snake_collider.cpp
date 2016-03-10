#include "snake_collider.h"

#include "snake_piece.h"

namespace acoross {
namespace snakebite {

void PlayerHeadCollider::Collide(SnakeBodyCollider & other, int cnt)
{
	int i = 1;
	i++;

	//owner_->container_.DeleteObject(owner_);

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

}
}
