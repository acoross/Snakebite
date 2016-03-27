#include "snake_npc_control_manager.h"

#include <chrono>
#include <algorithm>

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

static void changeDirection(std::default_random_engine& re, std::shared_ptr<GameSession> game_session, SnakeNpcControlManager::SetSnakeHandle& snakes, int64_t diff_in_ms)
{
	std::uniform_int_distribution<int> unin(0, 100);

	for (auto& snake_handle : snakes)
	{
		game_session->RequestToSnake(
			snake_handle,
			[p = unin(re)](Snake& snake)
		{
			if (p < 15) // 5 percent
			{
				snake.SetKeyDown(PK_LEFT);
			}
			else if (p < 30) // another 5 percent
			{
				snake.SetKeyDown(PK_RIGHT);
			}
			else if (p < 45)
			{
				snake.SetKeyUp(PK_LEFT);
				snake.SetKeyUp(PK_RIGHT);
			}
		});

		//if (auto snake = snake_wp.second.lock())
		//{
		//	int p = unin(re);
		//	if (p < 15) // 5 percent
		//	{
		//		snake->SetKeyDown(PK_LEFT);
		//	}
		//	else if (p < 30) // another 5 percent
		//	{
		//		snake->SetKeyDown(PK_RIGHT);
		//	}
		//	else if (p < 45)
		//	{
		//		snake->SetKeyUp(PK_LEFT);
		//		snake->SetKeyUp(PK_RIGHT);
		//	}
		//}
	}
}

SnakeNpcControlManager::SnakeNpcControlManager(std::weak_ptr<GameSession> game_session_wp)
	: game_session_wp_(game_session_wp)
{
	auto clock = std::chrono::high_resolution_clock();
	auto t = clock.now();
	random_engine_.seed((unsigned int)t.time_since_epoch().count());
}

void SnakeNpcControlManager::ChangeNpcDirection(int64_t diff_in_ms)
{
	std::lock_guard<std::recursive_mutex> lock(snake_npcs_mutex_);

	if (auto game_session = game_session_wp_.lock())
	// 임시:
	// 랜덤하게 방향을 변경.
	// UpdatteMove 가 불린 횟수와 관계없이,
	// 시간당 방향전환 횟수가 랜덤하도록 방향을 설정.
	if (checkChangeDirection(diff_in_ms))
	{
		changeDirection(random_engine_, game_session, snake_npc_handles_, diff_in_ms);
	}
}

Handle<Snake>::Type SnakeNpcControlManager::AddSnakeNpc()
{
	if (auto game_session = game_session_wp_.lock())
	{
		auto handle = game_session->AddSnake(
			[self = shared_from_this()](Snake& snake)
		{
			// 이러면 die 에서도 RemoveNpc 되기 때문에 RemoveNpc 가 두번 불릴걸...?
			self->UnregisterSnakeNpc(Handle<Snake>(snake).handle);
			self->AddSnakeNpc();	// 죽으면 다음꺼 생성
		});
				
		std::lock_guard<std::recursive_mutex> lock(snake_npcs_mutex_);
		snake_npc_handles_.emplace(handle);
		return handle;
	}
	
	return 0;
}

bool SnakeNpcControlManager::RemoveSnakeNpc(Handle<Snake>::Type handle)
{
	{
		std::lock_guard<std::recursive_mutex> lock(snake_npcs_mutex_);
		snake_npc_handles_.erase(handle);
	}
	
	if (auto game_session = game_session_wp_.lock())
	{
		return game_session->RemoveSnake(handle);
	}

	return false;
}

void SnakeNpcControlManager::UnregisterSnakeNpc(Handle<Snake>::Type handle)
{
	std::lock_guard<std::recursive_mutex> lock(snake_npcs_mutex_);
	snake_npc_handles_.erase(handle);
}

void SnakeNpcControlManager::RemoveFirstSnakeNpc()
{
	bool erase_snake = false;
	Handle<Snake>::Type handle = 0;
	{
		std::lock_guard<std::recursive_mutex> lock(snake_npcs_mutex_);
		auto it = snake_npc_handles_.begin();
		if (it != snake_npc_handles_.end())
		{
			handle = *it;
			snake_npc_handles_.erase(snake_npc_handles_.begin());
			erase_snake = true;
		}
	}

	if (erase_snake)
	{
		if (auto game_session = game_session_wp_.lock())
		{
			game_session->RemoveSnake(handle);
		}
	}
}

}//namespace snakebite
}//namespace acoross