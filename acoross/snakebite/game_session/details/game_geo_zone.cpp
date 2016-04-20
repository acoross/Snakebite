#include "game_geo_zone.h"

#include <memory>
#include <mutex>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "snake_collider.h"
#include "game_object.h"
#include "game_geo_zone_grid.h"

namespace acoross {
namespace snakebite {

void process_collision_2mapsnake(MapSnake& src_snakes, MapSnake& target_snakes, ListApple apples)
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
	int idx_zone_x, int idx_zone_y,
	MovingObjectContainer& game_boundary, 
	int left, int top, int width, int height)
	: strand_(io_service)
	, IDX_ZONE_X(idx_zone_x)
	, IDX_ZONE_Y(idx_zone_y)
	, game_boundary_(game_boundary)
	, zone_boundary_(left, width + left, top, top + height)
{
}

GameGeoZone::~GameGeoZone()
{
}

// update every snake position
void GameGeoZone::UpdateMove(int64_t diff_in_ms)
{
	shared_post(
		[this, diff_in_ms]()
	{
		for (auto& pair : snakes_)
		{
			auto& snake = pair.second;
			snake->UpdateMove(diff_in_ms, game_boundary_);
			process_collision_to_wall(snake);
		}
	}
	);
}

// process collision with neighbor zones
void GameGeoZone::ProcessCollision(GameGeoZoneGrid& neighbors_)
{
	shared_post(
		[this, &nb = neighbors_]()
	{
		auto shared_src_snakes = std::make_shared<MapSnake>(snakes_);

		for (auto& pair : *shared_src_snakes)
		{
			auto& snake = pair.second;
			auto& pos = snake->GetPosition();
			auto width = zone_boundary_.Width();
			auto height = zone_boundary_.Height();
			if (pos.x < zone_boundary_.Left - width / 4
				|| pos.x >= zone_boundary_.Right + width / 4
				|| pos.y < zone_boundary_.Top - height / 4
				|| pos.y >= zone_boundary_.Bottom + height / 4)
			{
				auto& dest_zone = nb.get_zone(pos.x, pos.y);
				if (dest_zone)
				{
					auto it = snakes_.find(pair.first);
					if (it != snakes_.end())
					{
						it->second->AtomicZoneIdx(0, 0);
						snakes_.erase(it);
						cached_snake_cnt_.fetch_sub(1);
					}
					dest_zone->AddSnake(snake);
				}
			}
		}

		for (int x = -1; x <= 1; ++x)
		{
			for (int y = -1; y <= 1; ++y)
			{
				auto neighbor_zone = nb.get_zone_by_idx(IDX_ZONE_X + x, IDX_ZONE_Y + y);
				if (neighbor_zone)
				{
					neighbor_zone->ProcessCollisionTo(shared_src_snakes);
				}
			}
		}
	});
}

// 다른 존하고의 충돌체크
void GameGeoZone::ProcessCollisionTo(
	std::shared_ptr<MapSnake> shared_other_snakes)
{
	shared_post(
		[this, shared_other_snakes]()
	{
		MapSnake src_snakes = snakes_;
		auto src_apples = apples_;

		if (src_snakes.empty() && src_apples.empty())
		{
			return;
		}

		process_collision_2mapsnake(
			*shared_other_snakes,
			src_snakes,
			src_apples);
	});
}

void GameGeoZone::AddApple(std::shared_ptr<Apple> apple)
{
	shared_post(
		[this, apple]()
	{
		apples_.emplace_back(apple);
		cached_apple_cnt_.fetch_add(1);
		apple->AtomicZoneIdx(IDX_ZONE_X, IDX_ZONE_Y);
	});
}

void GameGeoZone::RemoveApple(Apple* apple, std::function<void(bool result)> func)
{
	shared_post(
		[&, this, apple, func]()
	{
		for (auto it = apples_.begin(); it != apples_.end(); ++it)
		{
			if (it->get() == apple)
			{
				apple->AtomicZoneIdx(0, 0);

				apples_.erase(it);
				cached_apple_cnt_.fetch_sub(1);
				
				return func(true);
			}
		}

		return func(false);
	});
}

void GameGeoZone::AddSnake(std::shared_ptr<Snake> snake)
{
	shared_post(
		[this, snake]()
	{
		auto it = snakes_.emplace(Handle<Snake>(snake.get()).handle, snake);
		if (it.second)
		{
			it.first->second->AtomicZoneIdx(IDX_ZONE_X, IDX_ZONE_Y);
			cached_snake_cnt_.fetch_add(1);
		}
	});
}

void GameGeoZone::RemoveSnake(Handle<Snake>::Type snake)
{
	shared_post(
		[this, snake]()
	{
		auto it = snakes_.find(snake);
		if (it != snakes_.end())
		{
			it->second->AtomicZoneIdx(0, 0);
			snakes_.erase(it);
			cached_snake_cnt_.fetch_sub(1);
		}
	});
}

//serialized
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

void GameGeoZone::AsyncCloneSnakeList(CloneSnakelistCallback func)
{
	shared_post(
		[this, func]()
	{
		auto snakes = std::make_shared<CloneSnakelistT>();
		for (auto pair : snakes_)
		{
			snakes->push_back(std::make_pair(pair.first, pair.second->Clone()));
		}
		func(snakes);
	});
}

void GameGeoZone::AsyncCloneAppleList(CloneApplelistCallback func)
{
	shared_post(
		[this, func]()
	{
		auto apples = std::make_shared<CloneApplelistT>();
		for (auto apple : apples_)
		{
			apples->push_back(apple->Clone());
		}
		func(apples);
	});
}

size_t GameGeoZone::AppleCount() const
{
	return cached_apple_cnt_.load();
}

size_t GameGeoZone::SnakeCount() const
{
	return cached_snake_cnt_.load();
}

//void GameGeoZone::RequestEnterZone(std::shared_ptr<Snake> snake, int org_idx_zone_x, int org_idx_zone_y)
//{
//	EnteringInfo info{ snake, org_idx_zone_x, org_idx_zone_y };
//
//	std::lock_guard<std::recursive_mutex> lock(enter_leave_mutex_);
//	entering_snakes_.push(info);
//}
//
//void GameGeoZone::ProcessAllEnterZoneRequests()
//{
//	std::queue<EnteringInfo> enterings;
//	{
//		std::lock_guard<std::recursive_mutex> lock(enter_leave_mutex_);
//		enterings.swap(entering_snakes_);
//	}
//
//	while (!enterings.empty())
//	{
//		auto& info = enterings.front();
//		auto& pos = info.snake->GetPosition();
//		auto dest_zone = zone_grid_.get_zone(pos.x, pos.y);
//		if (dest_zone && dest_zone.get() == this)
//		{
//			AddSnake(info.snake);
//
//			auto src_zone = zone_grid_.get_zone(info.org_idx_zone_x, info.org_idx_zone_y);
//			if (src_zone)
//			{
//				//src_zone->RequestLeaveZone(info.snake, 0, 0);
//			}
//		}
//
//		enterings.pop();
//	}
//}
//
//void GameGeoZone::RequestLeaveZone(std::shared_ptr<Snake> snake, int org_idx_zone_x, int org_idx_zone_y)
//{
//	EnteringInfo info{ snake, org_idx_zone_x, org_idx_zone_y };
//
//	std::lock_guard<std::recursive_mutex> lock(enter_leave_mutex_);
//	leaving_snakes_.push(info);
//}
//
//void GameGeoZone::ProcessAllLeaaveRequests()
//{
//	std::queue<EnteringInfo> leavings;
//	{
//		std::lock_guard<std::recursive_mutex> lock(enter_leave_mutex_);
//		leavings.swap(leaving_snakes_);
//	}
//
//	while (!leavings.empty())
//	{
//		auto& info = leavings.front();
//		RemoveSnake(Handle<Snake>(info.snake.get()).handle);
//
//		leavings.pop();
//	}
//}
//void GameGeoZone::CheckAndRequestChangeZone()
//{
//	snakes_mutex_.lock();
//	auto snakes = snakes_;
//	snakes_mutex_.unlock();
//
//	// 전진
//	for (auto& snake : snakes)
//	{
//		auto& pos = snake.second->GetPosition();
//		if (pos.x < zone_boundary_.Left - zone_boundary_.Width() / 4. ||
//			pos.x >= zone_boundary_.Right + zone_boundary_.Width() / 4. ||
//			pos.y < zone_boundary_.Top - zone_boundary_.Height() / 4. ||
//			pos.y >= zone_boundary_.Bottom + zone_boundary_.Height() / 4.)
//		{
//			auto dest_zone = zone_grid_.get_zone(pos.x, pos.y);
//			if (dest_zone)
//			{
//				RemoveSnake(Handle<Snake>(snake.second.get()).handle);
//				dest_zone->RequestEnterZone(snake.second, IDX_ZONE_X, IDX_ZONE_Y);
//			}
//		}
//	}
//}

}
}