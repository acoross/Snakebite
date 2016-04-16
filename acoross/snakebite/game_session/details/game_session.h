#ifndef SNAKEBITE_GAME_SESSION_H_
#define SNAKEBITE_GAME_SESSION_H_

#include <list>
#include <map>
#include <queue>
#include <memory>
#include <utility>
#include <random>
#include <mutex>
#include <string>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "snake.h"
#include "apple.h"
#include "handle.h"

namespace acoross {
namespace snakebite {

// 맵, MovingObject 로 구성되는 하나의 게임 단위.
class GameSession final
{
public:
	using UpdateEventListner =
		std::function<void(const std::list<std::pair<Handle<Snake>::Type, GameObjectClone>>&, const std::list<GameObjectClone>&)>;
	using ListMovingObject = MovingObjectContainer::ListMovingObject;
	
	explicit GameSession(unsigned int init_apple_count = 20, int width = 500, int height = 500);
	~GameSession();

	// use lock
	void UpdateMove(int64_t diff_in_ms);
	void InvokeUpdateEvent();
	void ProcessCollisions();
	
	Handle<Snake>::Type AddSnake(std::string name = "noname", Snake::EventHandler onDieHandler = Snake::EventHandler());
	void AddApple();
	bool RemoveSnake(Handle<Snake>::Type snake);
	bool RemoveApple(Apple* apple);

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
	//

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

	void RequestToSnake(Handle<Snake>::Type handle, std::function<void(Snake&)> request)
	{
		auto it = snakes_.find(handle);
		if (it != snakes_.end())
		{
			request(*it->second.get());
		}
	}

	void AddUpdateEventListner(std::string name, UpdateEventListner on_update)
	{
		std::lock_guard<std::mutex> lock(update_listner_mutex_);
		on_update_event_listeners_[name] = on_update;
	}

	void UnregisterEventListner(std::string name)
	{
		std::lock_guard<std::mutex> lock(update_listner_mutex_);
		on_update_event_listeners_.erase(name);
	}

	//임시로 열어주는 API
	MovingObjectContainer& GetContainer() { return container_; }
	//

private:
	using MapSnake = std::map<Handle<Snake>::Type, SnakeSP>;
	using ListApple = std::list<AppleSP>;
	using CollisionMap = std::map<Handle<Snake>::Type, GameObjectWP>;
	using CollisionSet = std::set<Handle<Snake>::Type>;

	void ProcessCollisionToWall(SnakeSP actor);

	CollisionSet wall_collision_set_;
	CollisionMap collision_map_;
	MovingObjectContainer container_;

	std::default_random_engine random_engine_;

#pragma region snakes
	MapSnake snakes_;
	ListApple apples_;

	// 병신같지만 drawer 랑 동기화 하기 위해 추가한 lock 이다...
	// 수정 필요.
	std::recursive_mutex snakes_mutex_;
#pragma endregion snakes

	const double radius{ 5. };		// UNIT
	const double velocity{ 0.06 };	// UNIT/ms
	const Position2D player_pos{ 100, 100 };

	std::mutex update_listner_mutex_;
	std::map<std::string, UpdateEventListner> on_update_event_listeners_;

	//임시
	friend class LocalGameClient;
};

typedef std::shared_ptr<GameSession> GameSessionSP;
typedef std::weak_ptr<GameSession> GameSessionWP;

}
}
#endif //SNAKEBITE_GAME_SESSION_H_