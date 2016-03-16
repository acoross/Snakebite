#include "snake_npc_control_manager.h"

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

//static void changeDirection(std::default_random_engine& re, GameSession::MapSnakeWP& snakes, int64_t diff_in_ms)
//{
//	std::uniform_int_distribution<int> unin(0, 100);
//
//	for (auto& snake_wp : snakes)
//	{
//		if (auto snake = snake_wp.second.lock())
//		{
//			int p = unin(re);
//			if (p < 15) // 5 percent
//			{
//				auto ang_vel = snake->GetAngVelocity();
//				auto diff_ang = ang_vel * diff_in_ms;
//				snake->Turn(diff_ang);
//			}
//			else if (p < 30) // another 5 percent
//			{
//				auto ang_vel = snake->GetAngVelocity();
//				auto diff_ang = -ang_vel * diff_in_ms;
//				snake->Turn(diff_ang);
//			}
//		}
//	}
//}

void SnakeNpcControlManager::ChangeNpcDirection(int64_t diff_in_ms)
{
	std::lock_guard<std::recursive_mutex> lock(snake_npcs_mutex_);

	// 임시:
	// 랜덤하게 방향을 변경.
	// UpdatteMove 가 불린 횟수와 관계없이,
	// 시간당 방향전환 횟수가 랜덤하도록 방향을 설정.
	if (checkChangeDirection(diff_in_ms))
	{
		//changeDirection(random_engine_, snake_npcs_, diff_in_ms);
	}
}

std::weak_ptr<Snake> SnakeNpcControlManager::AddSnakeNpc()
{
	//auto snake = AddSnake();

	{
		std::lock_guard<std::recursive_mutex> lock(snake_npcs_mutex_);
		//snake_npcs_.emplace(snake.get(), snake);
	}
	//return snake;
	return std::weak_ptr<Snake>();
}

bool SnakeNpcControlManager::RemoveSnakeNpc(Snake * snake)
{
	std::lock_guard<std::recursive_mutex> lock(snake_npcs_mutex_);

	for (auto it = snake_npcs_.begin(); it != snake_npcs_.end(); ++it)
	{
		if (auto sp = it->second.lock())
		{
			if (sp.get() == snake)
			{
				snake_npcs_.erase(it);
				break;
			}
		}
	}

	//return RemoveSnake(snake);
	return false;
}

}//namespace snakebite
}//namespace acoross