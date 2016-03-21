#include "snake_npc_control_manager.h"

#include <chrono>
#include <algorithm>

#include <acoross/snakebite/game_session.h>

namespace acoross {
namespace snakebite {

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

static void changeDirection(std::default_random_engine& re, SnakeNpcControlManager::MapSnakeWP& snakes, int64_t diff_in_ms)
{
	std::uniform_int_distribution<int> unin(0, 100);

	for (auto& snake_wp : snakes)
	{
		if (auto snake = snake_wp.second.lock())
		{
			int p = unin(re);
			if (p < 15) // 5 percent
			{
				snake->SetKeyDown(PK_LEFT);
			}
			else if (p < 30) // another 5 percent
			{
				snake->SetKeyDown(PK_RIGHT);
			}
			else if (p < 45)
			{
				snake->SetKeyUp(PK_LEFT);
				snake->SetKeyUp(PK_RIGHT);
			}
		}
	}
}

SnakeNpcControlManager::SnakeNpcControlManager(GameSession& game_session)
	: game_session_(game_session)
{
	auto clock = std::chrono::high_resolution_clock();
	auto t = clock.now();
	random_engine_.seed((unsigned int)t.time_since_epoch().count());
}

void SnakeNpcControlManager::ChangeNpcDirection(int64_t diff_in_ms)
{
	std::lock_guard<std::recursive_mutex> lock(snake_npcs_mutex_);

	// 임시:
	// 랜덤하게 방향을 변경.
	// UpdatteMove 가 불린 횟수와 관계없이,
	// 시간당 방향전환 횟수가 랜덤하도록 방향을 설정.
	if (checkChangeDirection(diff_in_ms))
	{
		changeDirection(random_engine_, snake_npcs_, diff_in_ms);
	}
}

std::weak_ptr<Snake> SnakeNpcControlManager::AddSnakeNpc()
{
	auto snake = game_session_.AddSnake(
		/* onDie */
		[this_ = this](Snake& snake)
		{
			this_->RemoveSnakeNpc(&snake);
			this_->AddSnakeNpc();	// 죽으면 다음꺼 생성
		}
	);

	if (auto snakesp = snake.lock())
	{
		std::lock_guard<std::recursive_mutex> lock(snake_npcs_mutex_);
		snake_npcs_.emplace(snakesp.get(), snake);
	}
	return snake;
}

bool SnakeNpcControlManager::RemoveSnakeNpc(Snake* snake)
{
	{
		std::lock_guard<std::recursive_mutex> lock(snake_npcs_mutex_);
		snake_npcs_.erase(snake);
	}
	
	return game_session_.RemoveSnake(snake);
}

}//namespace snakebite
}//namespace acoross