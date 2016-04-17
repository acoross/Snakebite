#include "game_geo_zone.h"

#include <memory>
#include <mutex>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "snake_collider.h"
#include "game_object.h"
#include "game_geo_zone_grid.h"

namespace acoross {
namespace snakebite {

/////////////////////////////////////////////////
// GameGeoZone
GameGeoZone::GameGeoZone(
	GameGeoZoneGrid& owner_zone_grid,
	int idx_zone_x, int idx_zone_y,
	MovingObjectContainer& game_boundary, 
	int left, int top, int width, int height)
	: zone_grid_(owner_zone_grid)
	, IDX_ZONE_X(idx_zone_x)
	, IDX_ZONE_Y(idx_zone_y)
	, game_boundary_(game_boundary)
	, zone_boundary_(left, width + left, top, top + height)
{
}

GameGeoZone::~GameGeoZone()
{
}

void ProcessCollisionFor2Mapsnake(MapSnake& src_snakes, MapSnake& target_snakes, ListApple& apples)
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


// update every snake position

void GameGeoZone::UpdateMove(int64_t diff_in_ms)
{
	snakes_mutex_.lock();
	auto snakes = snakes_;
	snakes_mutex_.unlock();

	for (auto& snake : snakes)
	{
		snake.second->UpdateMove(diff_in_ms, game_boundary_);
		ProcessCollisionToWall(snake.second);
	}
}

void GameGeoZone::ProcessCollisions(
	std::map<std::pair<int, int>, std::pair<MapSnake, ListApple>>& zone_grid_objects)
{
	auto it_src = zone_grid_objects.find(std::make_pair(IDX_ZONE_X, IDX_ZONE_Y));
	if (it_src == zone_grid_objects.end())
	{
		return;
	}

	MapSnake& src_snakes = it_src->second.first;
	if (src_snakes.empty())
	{
		return;
	}

	// 이 geo zone 의 snakes 와
	// 주변 zone 의 snakes + apples 에 대해 충돌체크한다.
	// 주변 zone 의 범위: 주변 9칸
	for (int i = -1; i <= 1; ++i)
	{
		for (int j = -1; j <= 1; ++j)
		{
			if (IDX_ZONE_X + i < 0 
				|| IDX_ZONE_X + i >= zone_grid_.N_X
				|| IDX_ZONE_Y + j < 0
				|| IDX_ZONE_Y + j >= zone_grid_.N_Y)
			{
				continue;
			}

			auto it = zone_grid_objects.find(std::make_pair(IDX_ZONE_X, IDX_ZONE_Y));
			if (it != zone_grid_objects.end())
			{
				auto& target_snakes = it->second.first;
				auto& target_apples = it->second.second;

				ProcessCollisionFor2Mapsnake(
					src_snakes,
					target_snakes,
					target_apples);
			}
		}
	}
}


// 다른 존하고의 충돌체크

void GameGeoZone::ProcessCollisionToOtherZone(GameGeoZone & other_zone)
{
	auto& this_zone = *this;

	MapSnake& src_snakes = this_zone.GetSnakes();
	if (src_snakes.empty())
	{
		return;
	}

	auto& target_snakes = other_zone.GetSnakes();
	auto& target_apples = other_zone.GetApples();

	ProcessCollisionFor2Mapsnake(
		src_snakes,
		target_snakes,
		target_apples);
}

void GameGeoZone::ProcessCollisionToWall(SnakeSP actor)
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

bool GameGeoZone::AddApple(std::shared_ptr<Apple> apple)
{
	std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
	apples_.emplace_back(apple);
	return true;
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

bool GameGeoZone::AddSnake(std::shared_ptr<Snake> snake)
{
	std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
	auto it = snakes_.emplace(Handle<Snake>(snake.get()).handle, snake);
	return it.second;
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

std::list<std::pair<Handle<Snake>::Type, GameObjectClone>>
	GameGeoZone::CloneSnakeList()
{
	std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
	std::list<std::pair<Handle<Snake>::Type, GameObjectClone>> snakes;

	for (auto pair : snakes_)
	{
		snakes.push_back(std::make_pair(pair.first, pair.second->Clone()));
	}

	return snakes;
}

std::list<GameObjectClone> GameGeoZone::CloneAppleList()
{
	std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
	std::list<GameObjectClone> apples;

	for (auto apple : apples_)
	{
		apples.push_back(apple->Clone());
	}

	return apples;
}

size_t GameGeoZone::CalculateSnakeCount()
{
	std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
	return snakes_.size();
}

size_t GameGeoZone::CalculateAppleCount()
{
	std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
	return apples_.size();
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