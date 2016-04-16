#ifndef SNAKEBITE_GAME_GEO_ZONE_H_
#define SNAKEBITE_GAME_GEO_ZONE_H_

#include <mutex>

#include "snake.h"
#include "apple.h"
#include "handle.h"

namespace acoross {
namespace snakebite {

using MapSnake = std::map<Handle<Snake>::Type, SnakeSP>;
using ListApple = std::list<AppleSP>;
using CollisionMap = std::map<Handle<Snake>::Type, GameObjectWP>;
using CollisionSet = std::set<Handle<Snake>::Type>;

// 단일 게임을 여러개의 zone 으로 쪼개어 쓴다.
class GameGeoZone
{
public:
	explicit GameGeoZone(int left, int width, int top, int height);
	~GameGeoZone();

#pragma region use_snakes_mutex_
	// update every object in this zone
	void UpdateMove(int64_t diff_in_ms);
	// check collision and handle collsion event for every objects.
	void ProcessCollisions();

	void AddApple(std::shared_ptr<Apple> apple);
	bool RemoveApple(Apple* apple);

	Handle<Snake>::Type AddSnake(
		std::shared_ptr<Snake> snake,
		std::string name = "noname",
		Snake::EventHandler onDieHandler = Snake::EventHandler());
	bool RemoveSnake(Handle<Snake>::Type snake);
	auto CloneSnakeList()
	{
		std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
		std::list<std::pair<Handle<Snake>::Type, GameObjectClone>> snakes;

		for (auto pair : snakes_)
		{
			snakes.push_back(std::make_pair(pair.first, pair.second->Clone()));
		}

		return snakes;
	}
	std::list<GameObjectClone> CloneAppleList()
	{
		std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
		std::list<GameObjectClone> apples;

		for (auto apple : apples_)
		{
			apples.push_back(apple->Clone());
		}

		return apples;
	}
	size_t CalculateSnakeCount()
	{
		std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
		return snakes_.size();
	}
	size_t CalculateAppleCount()
	{
		std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
		return apples_.size();
	}
#pragma endregion use_snakes_mutex_

private:
	void ProcessArrivingBoundary(SnakeSP actor);

private:
#pragma region snakes - use_snakes_mutex_
	MapSnake snakes_;
	ListApple apples_;
	std::recursive_mutex snakes_mutex_;
#pragma endregion snakes - use_snakes_mutex_

	MovingObjectContainer boundary_;
};

}
}
#endif //SNAKEBITE_GAME_GEO_ZONE_H_