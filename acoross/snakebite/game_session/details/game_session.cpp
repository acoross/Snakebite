#include "game_session.h"

#include <cmath>
#include <memory>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <SDKDDKVer.h>
#include <boost/asio.hpp>

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
{}

void GameSession::StartZone(int frame_tick)
{
	zone_grid_.ProcessAllZone(
		[this](SbGeoZone& zone)->bool
	{
		list_zone_event_relayer_.emplace_back(
			std::make_pair(
				std::make_pair(zone.IDX_ZONE_X, zone.IDX_ZONE_Y), zone.GetUpdateEvent().make_relayer_up()));
		return true;
	});

	for (auto& zone_er : list_zone_event_relayer_)
	{
		zone_er.second->connect(
			[ix = zone_er.first.first, iy = zone_er.first.second, this]
		(SbGeoZone::UpdateEventData ed)
		{
			if (ix == ed.idx_x_ && iy == ed.idx_y_)	// 각 zone 은 옆 zone 정보까지 broadcast 하기 때문에, 전체 zone 정보를 수신하는 경우 filter 필요.
				return;

			this->InvokeUpdateEvent(ed);
		});
	}

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

void GameSession::AsyncAddSnakeTail(std::shared_ptr<SnakeNode> snake)
{
	auto zone = zone_grid_.get_zone(snake->GetPosition().x, snake->GetPosition().x);
	if (!zone)
	{
		_ASSERT(0);
		return;
	}

	strand_.post(
		[this, snake, zone]()
	{
		{
			zone->AsyncAddMovObj(snake);
		}
	});
}

Handle<Snake>::Type GameSession::AsyncMakeNewSnake(std::string name, Snake::EventHandler onDieHandler, bool is_connect_zone)
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
		, onDieHandler, name, is_connect_zone
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

		//FIXME!!!!!!! 임시로 이렇게 한거고.... 추후 수정 요망
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