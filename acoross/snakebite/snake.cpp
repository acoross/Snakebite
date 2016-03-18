#include "snake.h"
#include "snake_collider.h"
#include "game_session.h"

namespace acoross {
namespace snakebite {

Snake::Snake(GameSession& game_session, const Position2D & pos, double radius, 
	const Degree & angle, double velocity, double ang_vel, int len, 
	EventHandler onDie)
	: GameObject(game_session.GetContainer(), new SnakeCollider(this), pos, radius)
	, game_session_(game_session)
	, angle_(angle), velocity_(velocity), ang_vel_(ang_vel)
	, onDie_(onDie)
{
	for (int i = 0; i < len; ++i)
	{
		AddBody();
	}
}

Snake::~Snake()
{

}

void Snake::Move(const DirVector2D & diff_vec)
{
	const double dist_mov = diff_vec.Length();

	Position2D pos_prev_node = head_.GetPosition();
	head_.Move(diff_vec, container_);

	//DirVector2D diff_prev = diff_vec;

	for (auto& mo : body_list_)
	{
		const Position2D pos_current_node = mo.GetPosition();
		const DirVector2D diff_now_to_prev = {
			pos_prev_node.x - pos_current_node.x,
			pos_prev_node.y - pos_current_node.y
		};

		const double limitdist = 2 * (mo.GetRadius()) * 0.9;
		if (diff_now_to_prev.Length() < limitdist)
		{
			break;
		}

		auto diff_move = diff_now_to_prev.GetNormalized();
		diff_move.x *= dist_mov;
		diff_move.y *= dist_mov;

		pos_prev_node = mo.GetPosition();
		mo.Move(diff_move, container_);
	}
}

//@thread-safe: atomic once

void Snake::Die()
{
	bool expAlive = true;
	if (isAlive_.compare_exchange_strong(expAlive, false))
	{
		if (onDie_)
		{
			onDie_(*this);
		}

		game_session_.RemoveSnake(this);
	}
}

void Snake::AddBody()
{
	//container_.RegisterMovingObject(mo);
	body_list_.emplace_back(head_.GetPosition(), head_.GetRadius());
}

bool Snake::ProcessCollision(std::shared_ptr<GameObject> target)
{
	//TODO: !!!! collision_set_ 에 삽입된 snake 가 죽으면 맵에서 빠지지 않게 됨.
	// 하지만 그것 때문에 느려지는 건 아닌것 같고.
	if (IsCollidingTo(target))
	{
		//auto ret = collision_set_.insert(target.get());
		//if (ret.second == true)
		{
			// onCollideBegin
			//OnCollideStart(target.get());
			target->collider_->Collide(*this->collider_, 0);
		}
		//else
		{
			// onColliding
		}

		return true;
	}
	else
	{
		//if (collision_set_.erase(target.get()) > 0)
		{
			// onCollideEnd
		}

		return false;
	}
}

}
}