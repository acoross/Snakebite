#include "game_session.h"

#include <cmath>
#include <memory>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <SDKDDKVer.h>
#include <boost/asio.hpp>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "snake_collider.h"
#include "sb_zone_object.h"

namespace acoross {
namespace snakebite {

/////////////////////////////////////////////////
// GameSession
GameSession::GameSession(
	::boost::asio::io_service& io_service,
	int zone_width, int zone_height, int n_x, int n_y)
	: strand_(io_service)
	, zone_grid_(io_service, zone_width, zone_height, n_x, n_y)
{	
	auto clock = std::chrono::high_resolution_clock();
	auto t = clock.now();
	random_engine_.seed((unsigned int)t.time_since_epoch().count());
}

GameSession::~GameSession()
{
}

void GameSession::StartZone(int frame_tick)
{
	zone_grid_.ProcessAllZone(
		[this](auto& zone)->bool
	{
		zone.AsyncAddObserver(
			"GameSession",
			[&](int idx_zone_x, int idx_zone_y,
				SbGeoZone::SharedCloneZoneObjlistT snakes, 
				SbGeoZone::SharedCloneZoneObjlistT apples)
		{
			update_listner_mutex_.lock();
			auto event_listeners = on_update_event_listeners_;
			update_listner_mutex_.unlock();

			for (auto& pair : event_listeners)
			{
				auto& listner = pair.second;
				listner(idx_zone_x, idx_zone_y, *snakes, *apples);
			}
		});

		return true;
	});

	zone_grid_.ProcessAllZone(
		[frame_tick](auto& zone)->bool
	{
		zone.Run(frame_tick);
		return true;
	});
}

void GameSession::RequestRemoveApple(HandleT apple_handle, std::function<void(bool result)> func)
{
	zone_grid_.ProcessAllZone(
		[apple_handle, func](SbGeoZone& zone)->bool
	{
		zone.AsyncRemoveStaticObj(apple_handle, func);
		return true;
	}
	);
}

size_t GameSession::CalculateSnakeCount()
{
	size_t count = 0;
	zone_grid_.ProcessAllZone(
		[&count](SbGeoZone& zone)->bool
	{
		count += zone.AtomicMovObjCount();
		return true;
	}
	);
	return count;
}

size_t GameSession::CalculateAppleCount()
{
	size_t count = 0;
	zone_grid_.ProcessAllZone(
		[&count](SbGeoZone& zone)->bool
	{
		count += zone.AtomicStaticObjCount();
		return true;
	}
	);
	return count;
}

void GameSession::RequestToSnake(Handle<Snake>::Type handle, std::function<void(Snake&)> request)
{
	//std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
	strand_.post(
		[this, handle, request]()
	{
		auto it = snakes_.find(handle);
		if (it != snakes_.end())
		{
			request(*it->second.get());
		}
	});
}

Handle<Snake>::Type GameSession::AsyncMakeNewSnake(std::string name, Snake::EventHandler onDieHandler)
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
	if (!zone)
	{
		return Handle<Snake>::Type();
	}

	strand_.post(
		[this, snake, zone]()
	{
		auto it = snakes_.emplace(Handle<Snake>(snake.get()).handle, snake);
		if (it.second == true)
		{
			zone->AsyncAddMovObj(snake);
		}
	});
	
	return Handle<Snake>(snake.get()).handle;
}

void GameSession::AsyncRemoveSnake(Handle<Snake>::Type handle)
{
	strand_.post(
		[this, handle]()
	{
		//std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
		auto it = snakes_.find(handle);
		if (it != snakes_.end())
		{
			it->second->remove_this_from_zone_.store(true);
			snakes_.erase(it);
		}
	});
}

void GameSession::RequestMakeNewApple()
{
	auto& game_boundary = zone_grid_.GetBoundaryContainer();
	std::uniform_int_distribution<int> unin_x(game_boundary.Left, game_boundary.Right - 1);
	std::uniform_int_distribution<int> unin_y(game_boundary.Top, game_boundary.Bottom - 1);

	Position2D init_pos(unin_x(random_engine_), unin_y(random_engine_));

	auto apple = std::make_shared<Apple>(init_pos, radius * 2);
	auto zone = zone_grid_.get_zone(init_pos.x, init_pos.y);
	if (zone)
	{
		zone->AsyncAddStaticObj(apple);
	}
}

}
}