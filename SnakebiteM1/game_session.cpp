#include "stdafx.h"
#include "game_session.h"

#include <cmath>
#include <algorithm>
#include <chrono>
#include <random>

#include "geo_types.h"
#include "moving_object.h"

void acoross::snakebite::GameSession::Initialize()
{
	Position2D pos{ 200, 200 };
	Degree angle(0);
	double velocity{ 0.06 };	//  UNIT/ms
	double ang_vel{ 0.06 };		// degree/ms
	double radius{ 5. };		// UNIT
	
	for (int i = 0; i < 1; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			container_.AddNewMovingObject(pos, angle, velocity, ang_vel, 0.3 * radius * j + radius);
		}
	}
}

void acoross::snakebite::GameSession::CleanUp()
{
}


// 임시:
// 랜덤하게 방향을 변경.
// UpdatteMove 가 불린 횟수와 관계없이,
// 시간당 방향전환 횟수가 랜덤하도록 방향을 설정.
namespace acoross {
namespace snakebite {

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

static void changeDirection(GameSession::ListMovingObject& moving_objects, int64_t diff_in_ms)
{
	std::uniform_int_distribution<int> unin(0, 100);
	std::default_random_engine re;

	auto clock = std::chrono::high_resolution_clock();
	auto t = clock.now();

	re.seed((unsigned int)t.time_since_epoch().count());

	for (auto& mo : moving_objects)
	{
		int p = unin(re);
		if (p < 15) // 5 percent
		{
			auto ang_vel = mo->GetAngVelocity();
			auto diff_ang = ang_vel * diff_in_ms;
			mo->Turn((int)diff_ang);
		}
		else if (p < 30) // another 5 percent
		{
			auto ang_vel = mo->GetAngVelocity();
			auto diff_ang = -ang_vel * diff_in_ms;
			mo->Turn((int)diff_ang);
		}
	}
}

}
} //임시

void acoross::snakebite::GameSession::UpdateMove(int64_t diff_in_ms)
{
	// 임시:
	// 랜덤하게 방향을 변경.
	// UpdatteMove 가 불린 횟수와 관계없이,
	// 시간당 방향전환 횟수가 랜덤하도록 방향을 설정.
	if (checkChangeDirection())
	{
		changeDirection(container_.GetMovingObjects(), diff_in_ms);
	}
	
	// 전진
	for (auto& mo : container_.GetMovingObjects())
	{
		double diff_distance = mo->GetVelocity() * diff_in_ms;
		Position2D pos_now = mo->GetPosition();
		double angle_now_rad = mo->GetAngle().GetRad();
		
		DirVector2D diff_vec{
			diff_distance * std::cos(angle_now_rad),
			diff_distance * std::sin(angle_now_rad)
		};

		mo->Move(diff_vec);
	}
}