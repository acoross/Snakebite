#include "game_session.h"

#include <cmath>
#include <memory>
#include <mutex>
#include <chrono>
#include <algorithm>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "snake_collider.h"
#include "game_object.h"

namespace acoross {
namespace snakebite {

/////////////////////////////////////////////////
// GameSession
GameSession::GameSession(unsigned int init_apple_count, int width, int height)
	: container_(0, width, 0, height)	//container size
	, zone_(0, width, 0, height)
{	
	auto clock = std::chrono::high_resolution_clock();
	auto t = clock.now();
	random_engine_.seed((unsigned int)t.time_since_epoch().count());

	for (unsigned int i = 0; i < init_apple_count; ++i)
	{
		AddApple();
	}
}

GameSession::~GameSession()
{
}

void GameSession::UpdateMove(int64_t diff_in_ms)
{
	zone_.UpdateMove(diff_in_ms);
}

void GameSession::InvokeUpdateEvent()
{
	auto snake_list = zone_.CloneSnakeList();
	//auto snake_list = CloneSnakeList();
	auto apple_list = zone_.CloneAppleList();
	//auto apple_list = CloneAppleList();

	update_listner_mutex_.lock();
	auto event_listeners = on_update_event_listeners_;
	update_listner_mutex_.unlock();

	for (auto& pair : event_listeners)
	{
		auto& listner = pair.second;
		listner(snake_list, apple_list);
	}
}

void GameSession::ProcessCollisions()
{
	zone_.ProcessCollisions();
}


bool GameSession::RemoveApple(Apple * apple)
{
	return zone_.RemoveApple(apple);
}

Handle<Snake>::Type GameSession::AddSnake(std::string name, Snake::EventHandler onDieHandler)
{
	std::uniform_int_distribution<int> unin_x(container_.Left, container_.Right);
	std::uniform_int_distribution<int> unin_y(container_.Top, container_.Bottom);
	std::uniform_int_distribution<int> unin_degree(0, 360);

	Position2D init_pos(unin_x(random_engine_), unin_y(random_engine_));

	const double ang_vel{ 0.15 };		// degree/ms
	const int body_len{ 1 };

	auto snake = std::make_shared<Snake>(
	*this
	, init_pos, radius
	, unin_degree(random_engine_), velocity, ang_vel, body_len
	, onDieHandler, name);

	std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
	snakes_.emplace(Handle<Snake>(snake.get()).handle, snake);

	//
	return zone_.AddSnake(snake, name, onDieHandler);
}

bool GameSession::RemoveSnake(Handle<Snake>::Type snake)
{
	return zone_.RemoveSnake(snake);
}

void GameSession::AddApple()
{
	std::uniform_int_distribution<int> unin_x(container_.Left, container_.Right);
	std::uniform_int_distribution<int> unin_y(container_.Top, container_.Bottom);

	Position2D init_pos(unin_x(random_engine_), unin_y(random_engine_));

	auto apple = std::make_shared<Apple>(init_pos, radius * 2);

	/*{
		std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
		apples_.emplace_back(apple);
	}*/
	zone_.AddApple(apple);
}

void GameSession::ProcessCollisionToWall(SnakeSP actor)
{
	auto& pos = actor->GetPosition();
	if (pos.x <= container_.Left + 1 || pos.x >= container_.Right - 1
		|| pos.y <= container_.Top + 1 || pos.y >= container_.Bottom - 1)
	{
		auto ret = wall_collision_set_.insert(Handle<Snake>(actor.get()).handle);
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
		if (wall_collision_set_.erase(Handle<Snake>(actor.get()).handle) > 0)
		{
			// onCollideEnd
		}
	}
}

}
}