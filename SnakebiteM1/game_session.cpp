//#include "stdafx.h"
#include "game_session.h"

#include <cmath>
#include <algorithm>
#include <chrono>
#include <random>

#include "geo_types.h"
#include "moving_object.h"

namespace acoross {
namespace snakebite {

void GameSession::Initialize()
{
	Position2D pos(200, 200);
	Degree angle(0);
	double velocity{ 0.06 };	//  UNIT/ms
	double ang_vel{ 0.06 };		// degree/ms
	double radius{ 5. };		// UNIT
	
	int id = 0;
	for (int i = 0; i < 5; ++i)
	{
		for (int j = 0; j < 5; ++j)
		{
			double rad_to_set = 0.01 * radius * j + radius;
			SnakePiece* snake_head = new SnakePiece(
				container_.CreateMovingObject(id, pos, rad_to_set)
				, angle + 17 * j, velocity, ang_vel);

			for (int k = 0; k < 9; ++k)
			{
				snake_head->AddToTail(new SnakePiece(
					container_.CreateMovingObject(id, pos, rad_to_set)
					, angle, velocity, ang_vel));
			}

			snakes_.emplace_back(snake_head);
			++id;
		}
	}
}

void GameSession::CleanUp()
{
}


// 임시:
// 랜덤하게 방향을 변경.
// UpdatteMove 가 불린 횟수와 관계없이,
// 시간당 방향전환 횟수가 랜덤하도록 방향을 설정.
static bool checkChangeDirection()
{	
	auto clock = std::chrono::high_resolution_clock();
	auto t = clock.now();

	static auto last_time_ns = t.time_since_epoch();
	auto current_time_ns = t.time_since_epoch();
	
	if (current_time_ns.count() - last_time_ns.count() > 1000000 /*1s*/)
	{
		last_time_ns = current_time_ns;
		return true;
	}

	return false;
}

static void changeDirection(GameSession::ListSnakePiece& snakes, int64_t diff_in_ms)
{
	std::uniform_int_distribution<int> unin(0, 100);
	std::default_random_engine re;

	auto clock = std::chrono::high_resolution_clock();
	auto t = clock.now();

	re.seed((unsigned int)t.time_since_epoch().count());

	for (auto& snake : snakes)
	{
		int p = unin(re);
		if (p < 15) // 5 percent
		{
			auto ang_vel = snake->GetAngVelocity();
			auto diff_ang = ang_vel * diff_in_ms;
			snake->Turn((int)diff_ang);
		}
		else if (p < 30) // another 5 percent
		{
			auto ang_vel = snake->GetAngVelocity();
			auto diff_ang = -ang_vel * diff_in_ms;
			snake->Turn((int)diff_ang);
		}
	}
}

void GameSession::UpdateMove(int64_t diff_in_ms)
{
	// 임시:
	// 랜덤하게 방향을 변경.
	// UpdatteMove 가 불린 횟수와 관계없이,
	// 시간당 방향전환 횟수가 랜덤하도록 방향을 설정.
	if (checkChangeDirection())
	{
		changeDirection(snakes_, diff_in_ms);
	}
	
	// 전진
	for (auto& snake : snakes_)
	{
		double diff_distance = snake->GetVelocity() * diff_in_ms;
		Position2D pos_now = snake->GetMovingObject().GetPosition();
		double angle_now_rad = snake->GetAngle().GetRad();

		DirVector2D diff_vec{
			diff_distance * std::cos(angle_now_rad),
			diff_distance * std::sin(angle_now_rad)
		};

		snake->Move(diff_vec);
	}
}

}
}