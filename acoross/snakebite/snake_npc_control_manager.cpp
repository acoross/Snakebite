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
	}
}

SnakeNpcControlManager::SnakeNpcControlManager(
	::boost::asio::io_service& io_service, 
	std::weak_ptr<GameSession> game_session_wp)
	: strand_(io_service)
	, npc_change_dir_timer_(io_service)
	, game_session_wp_(game_session_wp)
{
	auto clock = std::chrono::high_resolution_clock();
	auto t = clock.now();
	random_engine_.seed((unsigned int)t.time_since_epoch().count());
}

void SnakeNpcControlManager::Start(int frametick)
{
	bool exp = false;
	if (is_started.compare_exchange_strong(exp, true))
	{
		AsyncChangeNpcDirection(frametick);
	}
}

void SnakeNpcControlManager::AsyncChangeNpcDirection(int64_t diff_in_ms)
{
	//std::lock_guard<std::recursive_mutex> lock(snake_npcs_mutex_);

	strand_.post(
		[this, diff_in_ms]()
	{
		npc_change_dir_timer_.expires_from_now(boost::posix_time::milliseconds(diff_in_ms));

		// 임시:
		// 랜덤하게 방향을 변경.
		// UpdatteMove 가 불린 횟수와 관계없이,
		// 시간당 방향전환 횟수가 랜덤하도록 방향을 설정.
		if (checkChangeDirection(diff_in_ms))
		{
			if (auto game_session = game_session_wp_.lock())
			{
				changeDirection(random_engine_, game_session, snake_npc_handles_, diff_in_ms);
			}
		}

		npc_change_dir_timer_.async_wait(
			[this, diff_in_ms](boost::system::error_code ec)
		{
			if (!ec)
			{
				AsyncChangeNpcDirection(diff_in_ms);
			}
		});
	});
}

void SnakeNpcControlManager::AsyncAddSnakeNpc()
{
	strand_.post(
		[this]()
	{
		if (auto game_session = game_session_wp_.lock())
		{
			auto handle = game_session->AsyncMakeNewSnake(
				"npc",
				[self = shared_from_this()](Snake& snake)
			{
				// 이러면 die 에서도 RemoveNpc 되기 때문에 RemoveNpc 가 두번 불릴걸...?
				self->async_unregister_snake_npc(Handle<Snake>(snake).handle);
				self->AsyncAddSnakeNpc();	// 죽으면 다음꺼 생성
			});

			//std::lock_guard<std::recursive_mutex> lock(snake_npcs_mutex_);
			snake_npc_handles_.emplace(handle);
		}
	});
}

void SnakeNpcControlManager::AsyncRemoveSnakeNpc(Handle<Snake>::Type handle)
{
	strand_.post(
		[this, handle]()
	{
		snake_npc_handles_.erase(handle);

		if (auto game_session = game_session_wp_.lock())
		{
			game_session->AsyncRemoveSnake(handle);
		}
	});
}

void SnakeNpcControlManager::async_unregister_snake_npc(Handle<Snake>::Type handle)
{
	//std::lock_guard<std::recursive_mutex> lock(snake_npcs_mutex_);

	strand_.post(
		[this, handle]()
	{
		snake_npc_handles_.erase(handle);
	});
	
}

void SnakeNpcControlManager::AsyncRemoveFirstSnakeNpc()
{
	strand_.post(
		[this]()
	{
		bool erase_snake = false;
		Handle<Snake>::Type handle = 0;

		//std::lock_guard<std::recursive_mutex> lock(snake_npcs_mutex_);
		auto it = snake_npc_handles_.begin();
		if (it != snake_npc_handles_.end())
		{
			handle = *it;
			snake_npc_handles_.erase(snake_npc_handles_.begin());
			erase_snake = true;
		}

		if (erase_snake)
		{
			if (auto game_session = game_session_wp_.lock())
			{
				game_session->AsyncRemoveSnake(handle);
			}
		}
	});
}

}//namespace snakebite
}//namespace acoross