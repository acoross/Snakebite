//#include "stdafx.h"
#include "game_session.h"

#include <cmath>
#include <algorithm>
#include <chrono>
#include <random>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "snake_collider.h"

namespace acoross {
namespace snakebite {

void GameSession::Initialize()
{
	Position2D pos(200, 200);
	Degree angle(0);
	double velocity{ 0.06 };	// UNIT/ms
	double ang_vel{ 0.1 };		// degree/ms
	double radius{ 5. };		// UNIT
	
	int id = 0;
	
	{
		Position2D player_pos(100, 100);

		double rad_to_set = radius;
		auto& snake_head = container_.CreateMovingObject<SnakePiece>(
			id, player_pos, rad_to_set
			, angle, velocity, 0.01
			, new PlayerHeadCollider());
		
		//new_mo.SetCollideCollback([cont = &container_, sh = snake_head, rad_to_set, id](MovingObject& other)->void
		//{
		//	if (other.GetId() == sh->GetMovingObject().GetId())
		//	{
		//		return;
		//	}
		//	crash++;			
		//	sh->AddToTail(new SnakePiece(
		//		cont->CreateMovingObject(id, Position2D(100, 100) /*tail 좌표로 이동된다.*/, rad_to_set)
		//		, 0, 0, 0));
		//});

		if (auto sh = snake_head.lock())
		{
			for (int k = 0; k < 9; ++k)
			{
				sh->AddToTail(container_.CreateMovingObject<SnakePiece>(
					id, player_pos, rad_to_set
					, angle, velocity, ang_vel
					, new SnakeBodyCollider()));
			}
		}

		snakes_.emplace_back(snake_head);
		++id;
	}

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			double rad_to_set = 0.01 * radius * j + radius;
			auto& snake_head = container_.CreateMovingObject<SnakePiece>(
				id, pos, rad_to_set
				, angle + 17 * j, velocity, ang_vel
				, new SnakeHeadCollider());

			if (auto sh = snake_head.lock())
			{
				for (int k = 0; k < 9; ++k)
				{
					sh->AddToTail(container_.CreateMovingObject<SnakePiece>(
						id, pos, rad_to_set
						, angle, velocity, ang_vel
						, new SnakeBodyCollider()));
				}
			}

			snakes_.emplace_back(snake_head);
			++id;
		}
	}
}

void GameSession::CleanUp()
{
	ListSnakePiece empty_snakes;
	empty_snakes.swap(snakes_);
	for (auto& snake : empty_snakes)
	{
		container_.DeleteObject(snake);
	}
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

static void changeDirection(GameSession::ListSnakePiece& snakes, int64_t diff_in_ms)
{
	std::uniform_int_distribution<int> unin(0, 100);
	std::default_random_engine re;

	auto clock = std::chrono::high_resolution_clock();
	auto t = clock.now();

	re.seed((unsigned int)t.time_since_epoch().count());

	for (auto& snake_wp : snakes)
	{
		if (auto snake = snake_wp.lock())
		{
			if (snake->GetId() == 0)
			{
				continue;
			}

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

void GameSession::UpdateMove(int64_t diff_in_ms)
{
	// 임시:
	// 랜덤하게 방향을 변경.
	// UpdatteMove 가 불린 횟수와 관계없이,
	// 시간당 방향전환 횟수가 랜덤하도록 방향을 설정.
	if (checkChangeDirection(diff_in_ms))
	{
		changeDirection(snakes_, diff_in_ms);
	}
	
	// 전진
	for (auto& snake_wp : snakes_)
	{
		if (auto snake = snake_wp.lock())
		{
			if (snake->GetId() == 0)
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
			}

			double diff_distance = snake->GetVelocity() * diff_in_ms;
			Position2D pos_now = snake->GetPosition();
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
}