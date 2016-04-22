#include "game_geo_zone.h"

#include <memory>
#include <mutex>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "snake_collider.h"
#include "game_object.h"
#include "game_geo_zone_grid.h"

namespace acoross {
namespace snakebite {

static void process_collision_2mapsnake(MapSnake& src_snakes, MapSnake& target_snakes, ListApple& apples)
{
	for (auto& snake1 : src_snakes)
	{
		for (auto& snake2 : target_snakes)
		{
			snake1.second->ProcessCollision(snake2.second);
		}

		for (auto& apple : apples)
		{
			snake1.second->ProcessCollision(apple);
		}
	}
}

/////////////////////////////////////////////////
// GameGeoZone
GameGeoZone::GameGeoZone(
	::boost::asio::io_service& io_service,
	GameGeoZoneGrid& owner_zone_grid,
	int idx_zone_x, int idx_zone_y,
	MovingObjectContainer& game_boundary, 
	int left, int top, int width, int height)
	: strand_(io_service)
	, owner_zone_grid_(owner_zone_grid)
	, zone_timer_(io_service)
	, IDX_ZONE_X(idx_zone_x)
	, IDX_ZONE_Y(idx_zone_y)
	, game_boundary_(game_boundary)
	, zone_boundary_(left, width + left, top, top + height)
{
}

GameGeoZone::~GameGeoZone()
{
}

void GameGeoZone::Run(int frame_tick)
{
	zone_timer_tick_ = frame_tick;

	bool exp = false;
	if (is_running_.compare_exchange_strong(exp, true))
	{
		do_update();
	}
}

void GameGeoZone::do_update()
{
	strand_.post(
		[this]()
	{
		zone_timer_.expires_from_now(boost::posix_time::milliseconds(zone_timer_tick_));

		update_movobj_position(zone_timer_tick_);
		process_collision(owner_zone_grid_);
		invoke_update_event_to_observers();

		zone_timer_.async_wait(
			[this](boost::system::error_code ec)
		{
			if (!ec)
			{
				do_update();
			}
		});
	});
}

void GameGeoZone::update_movobj_position(int64_t diff_in_ms)
{
	std::list<Handle<Snake>::Type> del_list;

	for (auto& pair : mov_objects_)
	{
		auto& snake = pair.second;

		if (snake->remove_this_from_zone_.load())
		{
			snake->AtomicZoneIdx(0, 0);
			cached_mov_object_cnt_.fetch_sub(1);
			del_list.push_back(pair.first);
		}
		else
		{
			snake->UpdateMove(diff_in_ms, game_boundary_);
			process_collision_to_wall(snake);
		}
	}

	for (auto& handle : del_list)
	{
		mov_objects_.erase(handle);
	}
}

void GameGeoZone::process_collision(GameGeoZoneGrid& neighbors_)
{
	auto shared_src_snakes = std::make_shared<MapSnake>(mov_objects_);
	for (auto& pair : *shared_src_snakes)
	{
		auto& snake = pair.second;
		auto& pos = snake->GetPosition();
		auto width = zone_boundary_.Width();
		auto height = zone_boundary_.Height();

		/*if (pos.x < zone_boundary_.Left - width / 4
		|| pos.x >= zone_boundary_.Right + width / 4
		|| pos.y < zone_boundary_.Top - height / 4
		|| pos.y >= zone_boundary_.Bottom + height / 4)*/
		if (pos.x < zone_boundary_.Left
			|| pos.x >= zone_boundary_.Right
			|| pos.y < zone_boundary_.Top
			|| pos.y >= zone_boundary_.Bottom)
		{
			auto& dest_zone = owner_zone_grid_.get_zone(pos.x, pos.y);
			if (dest_zone)
			{
				auto it = mov_objects_.find(pair.first);
				if (it != mov_objects_.end())
				{
					it->second->AtomicZoneIdx(0, 0);
					mov_objects_.erase(it);
					//del_list.push_back(pair.first);
					cached_mov_object_cnt_.fetch_sub(1);
				}
				dest_zone->AsyncAddMovObj(snake);
			}
		}
	}

	//// snake 를 한 존에서만 접근하려고 수정...
	this->AsyncProcessCollisionTo(shared_src_snakes);
	/*for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			auto neighbor_zone = neighbors_.get_zone_by_idx(IDX_ZONE_X + x, IDX_ZONE_Y + y);
			if (neighbor_zone)
			{
				neighbor_zone->AsyncProcessCollisionTo(shared_src_snakes);
			}
		}
	}*/
}

// 다른 존하고의 충돌체크
void GameGeoZone::AsyncProcessCollisionTo(
	std::shared_ptr<MapSnake> shared_other_snakes)
{
	strand_.post(
		[this, shared_other_snakes]()
	{
		if (mov_objects_.empty() && static_objects_.empty())
		{
			return;
		}

		process_collision_2mapsnake(
			*shared_other_snakes,
			mov_objects_,
			static_objects_);
	});
}

void GameGeoZone::AsyncAddObserver(std::string name, ObserverFuncT func)
{
	strand_.post(
		[this, name, func]()
	{
		observer_map_[name] = func;
	});
}

void GameGeoZone::AsyncAddStaticObj(std::shared_ptr<Apple> apple)
{
	strand_.post(
		[this, apple]()
	{
		static_objects_.emplace_back(apple);
		cached_static_object_cnt_.fetch_add(1);
		apple->AtomicZoneIdx(IDX_ZONE_X, IDX_ZONE_Y);
	});
}

void GameGeoZone::AsyncRemoveStaticObj(Apple* apple, std::function<void(bool result)> func)
{
	strand_.post(
		[&, this, apple, func]()
	{
		for (auto it = static_objects_.begin(); it != static_objects_.end(); ++it)
		{
			if (it->get() == apple)
			{
				apple->AtomicZoneIdx(0, 0);

				static_objects_.erase(it);
				cached_static_object_cnt_.fetch_sub(1);
				
				return func(true);
			}
		}

		return func(false);
	});
}

void GameGeoZone::AsyncAddMovObj(std::shared_ptr<Snake> snake)
{
	strand_.post(
		[this, snake]()
	{
		if (snake->remove_this_from_zone_.load())
		{
			snake->AtomicZoneIdx(0, 0);
			return;
		}

		auto it = mov_objects_.emplace(Handle<Snake>(snake.get()).handle, snake);
		if (it.second)
		{
			it.first->second->AtomicZoneIdx(IDX_ZONE_X, IDX_ZONE_Y);
			cached_mov_object_cnt_.fetch_add(1);
		}
	});
}

//void GameGeoZone::AsyncRemoveMovObj(Handle<Snake>::Type snake)
//{
//	strand_.post(
//		[this, snake]()
//	{
//		auto it = mov_objects_.find(snake);
//		if (it != mov_objects_.end())
//		{
//			it->second->AtomicZoneIdx(0, 0);
//			mov_objects_.erase(it);
//			cached_mov_object_cnt_.fetch_sub(1);
//		}
//	});
//}

//@need to be serialized
void GameGeoZone::process_collision_to_wall(SnakeSP actor)
{
	auto& pos = actor->GetPosition();
	
	if (pos.x <= game_boundary_.Left + 1 || pos.x >= game_boundary_.Right - 1
		|| pos.y <= game_boundary_.Top + 1 || pos.y >= game_boundary_.Bottom - 1)
	{
		auto ret = wall_collision_set_.insert(Handle<Snake>(actor.get()).handle);
		if (ret.second == true)
		{
			// onCollideBegin
			if (pos.x <= game_boundary_.Left + 1 || pos.x >= game_boundary_.Right - 1)
			{
				actor->SetAngle(180. - actor->GetAngle().Get());
			}
			else if (pos.y <= game_boundary_.Top + 1 || pos.y >= game_boundary_.Bottom - 1)
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

// zone 내부의 objects 위치를 broadcast

void GameGeoZone::invoke_update_event_to_observers()
{
	auto snakes = std::make_shared<CloneSnakelistT>();
	for (auto pair : mov_objects_)
	{
		if (pair.second->remove_this_from_zone_.load() == false)
		{
			snakes->push_back(std::make_pair(pair.first, pair.second->Clone()));
		}
	}

	auto apples = std::make_shared<CloneApplelistT>();
	for (auto apple : static_objects_)
	{
		apples->push_back(apple->Clone());
	}

	for (auto& pair : observer_map_)
	{
		pair.second(IDX_ZONE_X, IDX_ZONE_Y, snakes, apples);
	}
}
//
//void GameGeoZone::AsyncCloneMovObjList(CloneSnakelistCallback func)
//{
//	strand_.post(
//		[this, func]()
//	{
//		auto snakes = std::make_shared<CloneSnakelistT>();
//		for (auto pair : mov_objects_)
//		{
//			if (pair.second->remove_this_from_zone_.load() == false)
//			{
//				snakes->push_back(std::make_pair(pair.first, pair.second->Clone()));
//			}
//		}
//		func(snakes);
//	});
//}
//
//void GameGeoZone::AsyncCloneStaticObjList(CloneApplelistCallback func)
//{
//	strand_.post(
//		[this, func]()
//	{
//		auto apples = std::make_shared<CloneApplelistT>();
//		for (auto apple : static_objects_)
//		{
//			apples->push_back(apple->Clone());
//		}
//		func(apples);
//	});
//}

size_t GameGeoZone::AtomicStaticObjCount() const
{
	return cached_static_object_cnt_.load();
}

size_t GameGeoZone::AtomicMovObjCount() const
{
	return cached_mov_object_cnt_.load();
}

}
}