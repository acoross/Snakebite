#include "game_geo_zone.h"

//#include <cmath>
#include <memory>
#include <mutex>
//#include <chrono>
//#include <algorithm>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "snake_collider.h"
#include "game_object.h"

namespace acoross {
namespace snakebite {

static void updateMoveSnake(std::shared_ptr<Snake>& snake, int64_t diff_in_ms, MovingObjectContainer& container)
{
	// change direction
	{
		auto last_pk = snake->GetPlayerKey();
		if (last_pk == PK_RIGHT)
		{
			auto ang_vel = snake->GetAngVelocity();
			auto diff_ang = ang_vel * diff_in_ms;
			snake->Turn(diff_ang);
		}
		else if (last_pk == PK_LEFT)
		{
			auto ang_vel = snake->GetAngVelocity();
			auto diff_ang = -ang_vel * diff_in_ms;
			snake->Turn(diff_ang);
		}
	}

	// move to forward
	double diff_distance = snake->GetVelocity() * diff_in_ms;
	Position2D pos_now = snake->GetPosition();
	double angle_now_rad = snake->GetAngle().GetRad();

	DirVector2D diff_vec{
		diff_distance * std::cos(angle_now_rad),
		diff_distance * std::sin(angle_now_rad)
	};

	snake->Move(diff_vec, container);
}

/////////////////////////////////////////////////
// GameGeoZone
GameGeoZone::GameGeoZone(int left, int width, int top, int height)
	: boundary_(left, width, top, height)
{
}

GameGeoZone::~GameGeoZone()
{
}

void GameGeoZone::UpdateMove(int64_t diff_in_ms)
{
	//std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
	snakes_mutex_.lock();
	auto snakes = snakes_;
	snakes_mutex_.unlock();

	// ÀüÁø
	for (auto& snake : snakes)
	{
		acoross::snakebite::updateMoveSnake(snake.second, diff_in_ms, boundary_);
	}
}

void GameGeoZone::ProcessCollisions()
{
	snakes_mutex_.lock();
	MapSnake snakes = snakes_;
	ListApple apples = apples_;
	snakes_mutex_.unlock();

	for (auto& snake1 : snakes)
	{
		for (auto& snake2 : snakes)
		{
			snake1.second->ProcessCollision(snake2.second);
		}

		for (auto& apple : apples)
		{
			snake1.second->ProcessCollision(apple);
		}

		ProcessArrivingBoundary(snake1.second);
	}
}

void GameGeoZone::AddApple(std::shared_ptr<Apple> apple)
{
	std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
	apples_.emplace_back(apple);
}

bool GameGeoZone::RemoveApple(Apple * apple)
{
	std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
	for (auto it = apples_.begin(); it != apples_.end(); ++it)
	{
		if (it->get() == apple)
		{
			apples_.erase(it);
			return true;
		}
	}

	return false;
}

bool GameGeoZone::RemoveSnake(Handle<Snake>::Type snake)
{
	std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
	if (snakes_.erase(snake) > 0)
	{
		return true;
	}

	return false;
}

Handle<Snake>::Type GameGeoZone::AddSnake(std::shared_ptr<Snake> snake, std::string name, Snake::EventHandler onDieHandler)
{
	std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
	snakes_.emplace(Handle<Snake>(snake.get()).handle, snake);
	return Handle<Snake>(snake.get()).handle;
}

void GameGeoZone::ProcessArrivingBoundary(SnakeSP actor)
{
}

}
}