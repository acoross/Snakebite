#include "game_session.h"

#include <cmath>
#include <memory>
#include <mutex>
#include <chrono>
#include <algorithm>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "snake_collider.h"
#include "game_object.h"

namespace acoross {
namespace snakebite {


GameSession::GameSession(unsigned int init_apple_count, int width, int height)
	: container_(0, width, 0, height)	//container size
{	
	auto clock = std::chrono::high_resolution_clock();
	auto t = clock.now();
	random_engine_.seed((unsigned int)t.time_since_epoch().count());

	for (unsigned int i = 0; i < init_apple_count; ++i)
	{
		AddApple();
	}
}

GameSession::~GameSession()
{
}

static void updateMoveSnake(std::shared_ptr<Snake>& snake, int64_t diff_in_ms)
{
	// change direction
	{
		auto last_pk = snake->GetPlayerKey();
		if (last_pk == PK_RIGHT)
		{
			auto ang_vel = snake->GetAngVelocity();
			auto diff_ang = ang_vel * diff_in_ms;
			snake->Turn(diff_ang);
		}
		else if (last_pk == PK_LEFT)
		{
			auto ang_vel = snake->GetAngVelocity();
			auto diff_ang = -ang_vel * diff_in_ms;
			snake->Turn(diff_ang);
		}
	}

	// move to forward
	double diff_distance = snake->GetVelocity() * diff_in_ms;
	Position2D pos_now = snake->GetPosition();
	double angle_now_rad = snake->GetAngle().GetRad();

	DirVector2D diff_vec{
		diff_distance * std::cos(angle_now_rad),
		diff_distance * std::sin(angle_now_rad)
	};

	snake->Move(diff_vec);
}

void GameSession::UpdateMove(int64_t diff_in_ms)
{
	//std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
	snakes_mutex_.lock();
	auto snakes = snakes_;
	snakes_mutex_.unlock();

	// ÀüÁø
	for (auto& snake : snakes)
	{
		acoross::snakebite::updateMoveSnake(snake.second, diff_in_ms);
	}
}

void GameSession::ProcessCollisions()
{
	//std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
	snakes_mutex_.lock();
	MapSnake snakes = snakes_;
	ListApple apples = apples_;
	snakes_mutex_.unlock();

	for (auto& snake1 : snakes)
	{
		for (auto& snake2 : snakes)
		{
			snake1.second->ProcessCollision(snake2.second);
		}

		ProcessCollisionToWall(snake1.second);

		for (auto& apple : apples)
		{
			snake1.second->ProcessCollision(apple);
		}
	}
}

bool GameSession::RemoveSnake(Handle<Snake>::Type snake)
{
	std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);

	if (snakes_.erase(snake) > 0)
	{
		return true;
	}
	
	return false;
}

bool GameSession::RemoveApple(Apple * apple)
{
	std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);

	for (auto it = apples_.begin(); it != apples_.end(); ++it)
	{
		if (it->get() == apple)
		{
			apples_.erase(it);
			return true;
		}
	}

	return false;
}

SnakeWP GameSession::AddSnake_old(Snake::EventHandler onDieHandler, std::string name)
{
	std::uniform_int_distribution<int> unin_x(container_.Left, container_.Right);
	std::uniform_int_distribution<int> unin_y(container_.Top, container_.Bottom);
	std::uniform_int_distribution<int> unin_degree(0, 360);

	Position2D init_pos(unin_x(random_engine_), unin_y(random_engine_));

	const double ang_vel{ 0.15 };		// degree/ms
	const int body_len{ 1 };

	auto snake = std::make_shared<Snake>(
		*this
		, init_pos, radius
		, unin_degree(random_engine_), velocity, ang_vel, body_len
		, onDieHandler, name);

	{
		std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
		snakes_.emplace(Handle<Snake>(snake.get()).handle, snake);
	}
	return snake;
}

Handle<Snake>::Type GameSession::AddSnake(Snake::EventHandler onDieHandler, std::string name)
{
	std::uniform_int_distribution<int> unin_x(container_.Left, container_.Right);
	std::uniform_int_distribution<int> unin_y(container_.Top, container_.Bottom);
	std::uniform_int_distribution<int> unin_degree(0, 360);

	Position2D init_pos(unin_x(random_engine_), unin_y(random_engine_));

	const double ang_vel{ 0.15 };		// degree/ms
	const int body_len{ 1 };

	auto snake = std::make_shared<Snake>(
		*this
		, init_pos, radius
		, unin_degree(random_engine_), velocity, ang_vel, body_len
		, onDieHandler, name);

	{
		std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
		snakes_.emplace(Handle<Snake>(snake.get()).handle, snake);
	}
	return Handle<Snake>(snake.get()).handle;
}

void GameSession::AddApple()
{
	std::uniform_int_distribution<int> unin_x(container_.Left, container_.Right);
	std::uniform_int_distribution<int> unin_y(container_.Top, container_.Bottom);

	Position2D init_pos(unin_x(random_engine_), unin_y(random_engine_));

	auto apple = std::make_shared<Apple>(container_, init_pos, radius * 2);

	{
		std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
		apples_.emplace_back(apple);
	}
}

void GameSession::ProcessCollisionToWall(SnakeSP actor)
{
	auto& pos = actor->GetPosition();
	if (pos.x <= container_.Left + 1 || pos.x >= container_.Right - 1
		|| pos.y <= container_.Top + 1 || pos.y >= container_.Bottom - 1)
	{
		auto ret = wall_collision_set_.insert(Handle<Snake>(actor.get()).handle);
		if (ret.second == true)
		{
			// onCollideBegin
			if (pos.x <= container_.Left + 1 || pos.x >= container_.Right - 1)
			{
				actor->SetAngle(180. - actor->GetAngle().Get());
			}
			else if (pos.y <= container_.Top + 1 || pos.y >= container_.Bottom - 1)
			{
				actor->SetAngle(-1 * actor->GetAngle().Get());
			}
		}
		else
		{
			// onColliding
		}
	}
	else
	{
		if (wall_collision_set_.erase(Handle<Snake>(actor.get()).handle) > 0)
		{
			// onCollideEnd
		}
	}
}

}
}