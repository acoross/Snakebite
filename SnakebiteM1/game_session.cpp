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
	Position2D pos{ 0, 0 };
	Degree angle(0);
	double velocity{ 0.001 };
	double ang_vel{ 0.01 };
	moving_objects_.emplace_back(
		std::make_unique<MovingObject>(pos, angle, velocity, ang_vel));
}

void acoross::snakebite::GameSession::CleanUp()
{
}

namespace acoross {
namespace snakebite {

static bool checkChangeDirection()
{
	// 임시:
	// 랜덤하게 방향을 변경.
	// UpdatteMove 가 불린 횟수와 관계없이,
	// 시간당 방향전환 횟수가 랜덤하도록 방향을 설정.
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

	for (auto& mo : moving_objects)
	{
		int p = unin(re);
		if (p < 5) // 5 percent
		{
			auto ang_vel = mo->GetAngVelocity();
			auto diff_ang = ang_vel * diff_in_ms;
			mo->Turn((int)diff_ang);
		}
		else if (p < 10) // another 5 percent
		{
			auto ang_vel = mo->GetAngVelocity();
			auto diff_ang = -ang_vel * diff_in_ms;
			mo->Turn((int)diff_ang);
		}
	}
}

}
}

void acoross::snakebite::GameSession::UpdateMove(int64_t diff_in_ms)
{
	// 임시:
	// 랜덤하게 방향을 변경.
	// UpdatteMove 가 불린 횟수와 관계없이,
	// 시간당 방향전환 횟수가 랜덤하도록 방향을 설정.
	if (checkChangeDirection())
	{
		changeDirection(moving_objects_, diff_in_ms);
	}
	
	for (auto& mo : moving_objects_)
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