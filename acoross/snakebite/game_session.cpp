#include "game_session.h"

#include <cmath>
#include <algorithm>
#include <chrono>
#include <memory>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "snake_collider.h"

namespace acoross {
namespace snakebite {

void GameSession::Initialize()
{	
	auto clock = std::chrono::high_resolution_clock();
	auto t = clock.now();
	random_engine_.seed((unsigned int)t.time_since_epoch().count());

	Degree angle(0);
	double velocity{ 0.06 };	// UNIT/ms
	double ang_vel{ 1.5 };		// degree/ms
	double radius{ 5. };		// UNIT
	
	Position2D player_pos(100, 100);
	double rad_to_set = radius;

	player_ = std::make_shared<Snake>(container_, player_pos, rad_to_set, angle, velocity, 0.15, 9);

	std::uniform_int_distribution<int> unin_x(container_.Left, container_.Right);
	std::uniform_int_distribution<int> unin_y(container_.Top, container_.Bottom);

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			Position2D init_pos(unin_x(random_engine_), unin_y(random_engine_));
			double rad_to_set = 0.01 * radius * j + radius;

			auto snake = std::make_shared<Snake>(
				container_, init_pos, rad_to_set
				, angle + 17 * j, velocity, ang_vel, 7);
			
			snakes_.emplace_back(snake);
		}
	}

	for (int i = 0; i < 10; ++i)
	{
		Position2D init_pos(unin_x(random_engine_), unin_y(random_engine_));
		auto apple = std::make_shared<Apple>(container_, init_pos, radius * 2);

		apples_.emplace_back(apple);
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

static void changeDirection(std::default_random_engine& re, GameSession::ListSnake& snakes, int64_t diff_in_ms)
{
	std::uniform_int_distribution<int> unin(0, 100);

	for (auto& snake : snakes)
	{
		//if (auto snake = snake_wp.lock())
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
	if (auto snake = player_)//.lock())
	{
		if (last_pk_ == PK_RIGHT)
		{
			auto ang_vel = snake->GetAngVelocity();
			auto diff_ang = ang_vel * diff_in_ms;
			snake->Turn(diff_ang);
		}
		else if (last_pk_ == PK_LEFT)
		{
			auto ang_vel = snake->GetAngVelocity();
			auto diff_ang = -ang_vel * diff_in_ms;
			snake->Turn(diff_ang);
		}

		acoross::snakebite::updateMoveSnake(snake, diff_in_ms);
	}
	
	// 임시:
	// 랜덤하게 방향을 변경.
	// UpdatteMove 가 불린 횟수와 관계없이,
	// 시간당 방향전환 횟수가 랜덤하도록 방향을 설정.
	if (checkChangeDirection(diff_in_ms))
	{
		changeDirection(random_engine_, snakes_, diff_in_ms);
	}
	
	// 전진
	for (auto& snake : snakes_)
	{
		acoross::snakebite::updateMoveSnake(snake, diff_in_ms);
	}
}

void GameSession::ProcessCollisions()
{
	//container_.ProcessCollisions();

	ListSnake snakes = snakes_;
	snakes.push_back(player_);

	for (auto& snake1 : snakes)
	{
		for (auto& snake2 : snakes)
		{
			ProcessCollision(snake1, snake2);
		}

		ProcessCollisionToWall(snake1);
	}
}

void GameSession::ProcessCollision(std::shared_ptr<Snake> actor, std::shared_ptr<Snake> target)
{
	if (actor.get() == target.get())
		return;

	if (actor->IsCollidingTo(target))
	{
		auto ret = collision_map_.insert(CollisionMap::value_type(actor.get(), SnakeWP(target)));
		if (ret.second == true)
		{
			// onCollideBegin
			actor->OnCollideStart(target.get());
		}
		else
		{
			// onColliding
			actor->OnColliding(target.get());
		}
	}
	else
	{
		if (collision_map_.erase(actor.get()) > 0)
		{
			// onCollideEnd
			actor->OnCollideEnd(target.get());
		}
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