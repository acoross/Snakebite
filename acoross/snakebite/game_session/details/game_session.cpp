#include "game_session.h"

#include <cmath>
#include <memory>
#include <mutex>
#include <chrono>
#include <algorithm>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "snake_collider.h"
#include "game_object.h"
#include "game_geo_zone.h"

namespace acoross {
namespace snakebite {

/////////////////////////////////////////////////
// GameSession
GameSession::GameSession(
	unsigned int init_apple_count, int zone_width, int zone_height, int n_x, int n_y)
	: zone_grid_(zone_width, zone_height, n_x, n_y)
{	
	auto clock = std::chrono::high_resolution_clock();
	auto t = clock.now();
	random_engine_.seed((unsigned int)t.time_since_epoch().count());

	for (unsigned int i = 0; i < init_apple_count; ++i)
	{
		MakeNewApple();
	}
}

GameSession::~GameSession()
{
}

void GameSession::UpdateMove(int64_t diff_in_ms)
{
	zone_grid_.ProcessAllZone(
		[&game_boundary = zone_grid_.GetBoundaryContainer(), 
		diff_in_ms](auto& zone)->bool
	{
		zone.UpdateMove(diff_in_ms);
		//zone.CheckAndRequestChangeZone();
		return true;
	});
}

void GameSession::InvokeUpdateEvent()
{
	std::list<std::pair<Handle<Snake>::Type, GameObjectClone>> snakes;
	std::list<GameObjectClone> apples;
	zone_grid_.ProcessAllZone(
		[&snakes, &apples](GameGeoZone& zone)->bool
	{
		auto snake_list = zone.CloneSnakeList();
		snakes.insert(snakes.end(), snake_list.begin(), snake_list.end());

		auto apple_list = zone.CloneAppleList();
		apples.insert(apples.end(), apple_list.begin(), apple_list.end());
		
		return true;
	}
	);

	update_listner_mutex_.lock();
	auto event_listeners = on_update_event_listeners_;
	update_listner_mutex_.unlock();

	for (auto& pair : event_listeners)
	{
		auto& listner = pair.second;
		listner(snakes, apples);
	}
}

void GameSession::ProcessCollisions()
{
	zone_grid_.ProcessAllZone(
		[](GameGeoZone& zone)->bool
		{
			/*zone.ProcessAllEnterZoneRequests();
			zone.ProcessAllLeaaveRequests();*/
			return true;
		});

	std::map<std::pair<int, int>, std::pair<MapSnake, ListApple>> zone_grid_objects;
	zone_grid_.ProcessAllZone(
		[&zone_grid_objects](GameGeoZone& zone)->bool
		{
			zone_grid_objects[std::make_pair(zone.IDX_ZONE_X, zone.IDX_ZONE_Y)]
				= std::make_pair(zone.GetSnakes(), zone.GetApples());
			return true;
		});

	zone_grid_.ProcessAllZone(
		[&zone_grid_objects](GameGeoZone& zone)->bool
		{
			zone.ProcessCollisions(zone_grid_objects);
			return true;
		});
}

bool GameSession::RemoveApple(Apple * apple)
{
	return zone_grid_.ProcessAllZone(
		[apple](GameGeoZone& zone)->bool
	{
		return zone.RemoveApple(apple);
	}
		, true	// stop when success
	);
}

size_t GameSession::CalculateSnakeCount()
{
	size_t count = 0;
	zone_grid_.ProcessAllZone(
		[&count](GameGeoZone& zone)->bool
	{
		count += zone.CalculateSnakeCount();
		return true;
	}
	);
	return count;
}

size_t GameSession::CalculateAppleCount()
{
	size_t count = 0;
	zone_grid_.ProcessAllZone(
		[&count](GameGeoZone& zone)->bool
	{
		count += zone.CalculateAppleCount();
		return true;
	}
	);
	return count;
}

void GameSession::RequestToSnake(Handle<Snake>::Type handle, std::function<void(Snake&)> request)
{
	zone_grid_.ProcessAllZone(
		[handle, request](GameGeoZone& zone)->bool
	{
		auto snake = zone.GetSnakes();

		auto it = snake.find(handle);
		if (it != snake.end())
		{
			request(*it->second.get());
		}
		return true;
	}
	);
}

Handle<Snake>::Type GameSession::MakeNewSnake(std::string name, Snake::EventHandler onDieHandler)
{
	auto& game_boundary = zone_grid_.GetBoundaryContainer();
	std::uniform_int_distribution<int> unin_x(game_boundary.Left, game_boundary.Right - 1);
	std::uniform_int_distribution<int> unin_y(game_boundary.Top, game_boundary.Bottom - 1);
	std::uniform_int_distribution<int> unin_degree(0, 360);

	Position2D init_pos(unin_x(random_engine_), unin_y(random_engine_));

	const double ang_vel{ 0.15 };		// degree/ms
	const int body_len{ 1 };

	auto snake = std::make_shared<Snake>(
			*this
			, init_pos, radius
			, unin_degree(random_engine_), velocity, ang_vel, body_len
			, onDieHandler, name
			);

	auto zone = zone_grid_.get_zone(snake->GetPosition().x, snake->GetPosition().x);
	if (zone)
	{
		return zone->AddSnake(snake);
	}

	return Handle<Snake>::Type(0);
}

bool GameSession::RemoveSnake(Handle<Snake>::Type snake)
{
	return zone_grid_.ProcessAllZone(
		[snake](GameGeoZone& zone)
		{
			return zone.RemoveSnake(snake);
		}, 
		true
		);
}

void GameSession::MakeNewApple()
{
	auto& game_boundary = zone_grid_.GetBoundaryContainer();
	std::uniform_int_distribution<int> unin_x(game_boundary.Left, game_boundary.Right - 1);
	std::uniform_int_distribution<int> unin_y(game_boundary.Top, game_boundary.Bottom - 1);

	Position2D init_pos(unin_x(random_engine_), unin_y(random_engine_));

	auto apple = std::make_shared<Apple>(init_pos, radius * 2);

	/*{
		std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
		apples_.emplace_back(apple);
	}*/
	auto zone = zone_grid_.get_zone(init_pos.x, init_pos.y);
	if (zone)
	{
		zone->AddApple(apple);
	}
	//zone_.AddApple(apple);
}

}
}