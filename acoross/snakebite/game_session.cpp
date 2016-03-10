//#include "stdafx.h"
#include "game_session.h"

#include <cmath>
#include <algorithm>
#include <chrono>
#include <random>
#include <memory>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "snake_collider.h"

namespace acoross {
namespace snakebite {

void GameSession::Initialize()
{
	Degree angle(0);
	double velocity{ 0.06 };	// UNIT/ms
	double ang_vel{ 0.1 };		// degree/ms
	double radius{ 5. };		// UNIT
	
	Position2D player_pos(100, 100);
	double rad_to_set = radius;

	player_ = std::make_shared<Snake>(container_, player_pos, rad_to_set, angle, velocity, 0.01, 10);

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			double rad_to_set = 0.01 * radius * j + radius;
			auto snake = std::make_shared<Snake>(container_, player_pos, rad_to_set
				, angle + 17 * j, velocity, ang_vel, 10);
			
			snakes_.emplace_back(snake);
		}
	}
}

void GameSession::CleanUp()
{
	ListSnake empty_snakes;
	empty_snakes.swap(snakes_);
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

static void changeDirection(GameSession::ListSnake& snakes, int64_t diff_in_ms)
{
	std::uniform_int_distribution<int> unin(0, 100);
	std::default_random_engine re;

	auto clock = std::chrono::high_resolution_clock();
	auto t = clock.now();

	re.seed((unsigned int)t.time_since_epoch().count());

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
		changeDirection(snakes_, diff_in_ms);
	}
	
	// 전진
	for (auto& snake : snakes_)
	{
		acoross::snakebite::updateMoveSnake(snake, diff_in_ms);
	}
}

}
}