#ifndef SNAKEBITE_GAME_SESSION_H_
#define SNAKEBITE_GAME_SESSION_H_

#include <list>
#include <unordered_map>
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
#include "game_geo_zone_grid.h"

namespace acoross {
namespace snakebite {

// 맵, GameObject 로 구성되는 하나의 게임 단위.
class GameSession final
{
public:
	using UpdateEventListner =
		std::function<void(int idx_x, int idx_y, 
			const std::list<std::pair<Handle<Snake>::Type, ZoneObjectClone>>&, 
			const std::list<ZoneObjectClone>&)>;
	using ListMovingObject = MovingObjectContainer::ListMovingObject;
	
	explicit GameSession(
		::boost::asio::io_service& io_service,
		int zone_width, int zone_height, int n_x, int n_y);
	~GameSession();

	// update every object in this zone
	void UpdateMove(int64_t diff_in_ms);
	// clone every object and invoke listening update event handlers.
	void InvokeUpdateEvent();
	// check collision and handle collsion event for every objects.
	void ProcessCollisions();
	
	Handle<Snake>::Type MakeNewSnake(
		std::string name = "noname", 
		Snake::EventHandler onDieHandler = Snake::EventHandler());
	void MakeNewApple();
	bool RemoveSnake(Handle<Snake>::Type snake);
	void RemoveApple(Apple* apple, std::function<void(bool result)> func);

	size_t CalculateSnakeCount();
	size_t CalculateAppleCount();

	void RequestToSnake(Handle<Snake>::Type handle, std::function<void(Snake&)> request);

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

private:
	std::default_random_engine random_engine_;

//#pragma region snakes - use_snakes_mutex_
	MapSnake snakes_;
	//ListApple apples_;
	std::recursive_mutex snakes_mutex_;
//#pragma endregion snakes - use_snakes_mutex_

	// to make new snake...
	const double radius{ 5. };		// UNIT
	const double velocity{ 0.06 };	// UNIT/ms
	const Position2D player_pos{ 100, 100 };

	std::mutex update_listner_mutex_;
	std::unordered_map<std::string, UpdateEventListner> on_update_event_listeners_;

	GameGeoZoneGrid zone_grid_;

	//임시
	friend class LocalGameClient;
};

typedef std::shared_ptr<GameSession> GameSessionSP;
typedef std::weak_ptr<GameSession> GameSessionWP;

}
}
#endif //SNAKEBITE_GAME_SESSION_H_