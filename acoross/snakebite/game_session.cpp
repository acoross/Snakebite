#include "game_session.h"

#include <cmath>
#include <algorithm>
#include <chrono>
#include <memory>
#include <mutex>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "snake_collider.h"
#include "game_object.h"

namespace acoross {
namespace snakebite {

void GameSession::Initialize()
{	
	auto clock = std::chrono::high_resolution_clock();
	auto t = clock.now();
	random_engine_.seed((unsigned int)t.time_since_epoch().count());

	//InitPlayer();

	for (int i = 0; i < 1; ++i)
	{
		AddSnake();
	}

	for (int i = 0; i < 20; ++i)
	{
		AddApple();
	}
}

void GameSession::CleanUp()
{
}


// 임시:
// 랜덤하게 방향을 변경.
// UpdatteMove 가 불린 횟수와 관계없이,
// 시간당 방향전환 횟수가 랜덤하도록 방향을 설정.
static bool checkChangeDirection(int64_t diff_in_ms)
{	
	static int64_t delay_sum = 0;
	
	delay_sum += diff_in_ms;
	if (delay_sum > 100 /*1s*/)
	{
		delay_sum = 0;
		return true;
	}

	return false;
}

static void changeDirection(std::default_random_engine& re, GameSession::ListSnakeNpc& snakes, int64_t diff_in_ms)
{
	std::uniform_int_distribution<int> unin(0, 100);

	for (auto& snake_wp : snakes)
	{
		if (auto snake = snake_wp.lock())
		{
			int p = unin(re);
			if (p < 15) // 5 percent
			{
				auto ang_vel = snake->GetAngVelocity();
				auto diff_ang = ang_vel * diff_in_ms;
				snake->Turn(diff_ang);
			}
			else if (p < 30) // another 5 percent
			{
				auto ang_vel = snake->GetAngVelocity();
				auto diff_ang = -ang_vel * diff_in_ms;
				snake->Turn(diff_ang);
			}
		}
	}
}

static void updateMoveSnake(std::shared_ptr<Snake>& snake, int64_t diff_in_ms)
{
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
	std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);

	if (auto snake = player_.lock())
	{
		auto last_pk = GetPlayerKey();
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
	
	// 임시:
	// 랜덤하게 방향을 변경.
	// UpdatteMove 가 불린 횟수와 관계없이,
	// 시간당 방향전환 횟수가 랜덤하도록 방향을 설정.
	if (checkChangeDirection(diff_in_ms))
	{
		changeDirection(random_engine_, snake_npcs_, diff_in_ms);
	}
	
	// 전진
	for (auto& snake : snakes_)
	{
		acoross::snakebite::updateMoveSnake(snake, diff_in_ms);
	}
}

void GameSession::ProcessCollisions()
{
	std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
	
	ListSnake snakes = snakes_;
	ListApple apples = apples_;

	for (auto& snake1 : snakes)
	{
		for (auto& snake2 : snakes)
		{
			snake1->ProcessCollision(snake2);
		}

		ProcessCollisionToWall(snake1);

		for (auto& apple : apples)
		{
			snake1->ProcessCollision(apple);
		}
	}
}

void GameSession::RemoveSnake(Snake * snake)
{
	std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);

	for (auto it = snakes_.begin(); it != snakes_.end(); ++it)
	{
		if (it->get() == snake)
		{
			snakes_.erase(it);
			return;
		}
	}
}

void GameSession::RemoveApple(Apple * apple)
{
	std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);

	for (auto it = apples_.begin(); it != apples_.end(); ++it)
	{
		if (it->get() == apple)
		{
			apples_.erase(it);
			return;
		}
	}
}

std::shared_ptr<Snake> GameSession::AddSnake()
{
	std::uniform_int_distribution<int> unin_x(container_.Left, container_.Right);
	std::uniform_int_distribution<int> unin_y(container_.Top, container_.Bottom);
	std::uniform_int_distribution<int> unin_degree(0, 360);

	Position2D init_pos(unin_x(random_engine_), unin_y(random_engine_));
	
	const double ang_vel{ 1.5 };		// degree/ms
	const int body_len{ 1 };

	auto snake = std::make_shared<Snake>(
		*this
		, container_, init_pos, radius
		, unin_degree(random_engine_), velocity, ang_vel, body_len);

	{
		std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
		snake_npcs_.push_back(snake);
		snakes_.emplace_back(snake);
	}

	return snake;
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

void GameSession::InitPlayer()
{
	auto player = std::make_shared<Snake>(*this, container_, player_pos, radius, 0, velocity, 0.15, 2);
	if (auto player = player_.lock())
	{
		RemoveSnake(player.get());
	}
	
	{
		std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
		player_ = player;
		snakes_.push_back(player);
	}
}

void GameSession::ProcessCollisionToWall(std::shared_ptr<Snake> actor)
{
	auto& pos = actor->GetPosition();
	if (pos.x <= container_.Left + 1 || pos.x >= container_.Right - 1
		|| pos.y <= container_.Top + 1 || pos.y >= container_.Bottom - 1)
	{
		auto ret = wall_collision_set_.insert(actor.get());
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
		if (wall_collision_set_.erase(actor.get()) > 0)
		{
			// onCollideEnd
		}
	}
}

}
}