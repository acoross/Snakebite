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

#include <acoross/snakebite/zone_system/details/signal.h>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "sb_zone_object.h"
#include "snake.h"
#include "apple.h"

namespace acoross {
namespace snakebite {

using MapSnake = std::unordered_map<Handle<Snake>::Type, std::shared_ptr<Snake>>;
using ListApple = std::list<AppleSP>;

// 맵, GameObject 로 구성되는 하나의 게임 단위.
class GameSession final
{
public:
	using UpdateEventHandler = void(int, int,
		SbGeoZone::CloneZoneObjListT&, SbGeoZone::CloneZoneObjListT&);
	using UpdateEventListner = std::function<UpdateEventHandler>;
	using ListMovingObject = MovingObjectContainer::ListMovingObject;

	explicit GameSession(
		::boost::asio::io_service& io_service,
		int zone_width, int zone_height, int n_x, int n_y);
	~GameSession();

	void StartZone(int frame_tick);

	Handle<Snake>::Type AsyncMakeNewSnake(
		std::string name = "noname",
		Snake::EventHandler onDieHandler = Snake::EventHandler(),
		bool is_connect_zone = false);
	void RequestMakeNewApple();
	void AsyncRemoveSnake(Handle<Snake>::Type snake);
	void RequestRemoveApple(HandleT apple_handle, std::function<void(bool result)> func);

	void AsyncAddSnakeTail(std::shared_ptr<SnakeNode> snake);

	size_t CalculateSnakeCount();
	size_t CalculateAppleCount();

	// 이 함수는 refactoring 필요.
	void RequestToSnake(Handle<Snake>::Type handle,
		std::function<void(Snake&)> request)
	{
		//std::lock_guard<std::recursive_mutex> lock(snakes_mutex_);
		strand_.post(
			[this, handle, request]()
		{
			auto it = snakes_.find(handle);
			if (it != snakes_.end())
			{
				return request(*it->second.get());
			}
		});
	}

	auto MakeConnectionToUpdateEvent(SbGeoZone::ObserverT on_update)
	{
		return update_event_.connect(on_update);
	}

	void InvokeUpdateEvent(int idx_zone_x, int idx_zone_y,
		SbGeoZone::SharedCloneZoneObjlistT snakes, SbGeoZone::SharedCloneZoneObjlistT apples)
	{
		update_event_.invoke(idx_zone_x, idx_zone_y, snakes, apples);
	}

	const SbGeoZoneGrid& GetZoneGrid() const
	{
		return zone_grid_;
	}

private:
	::boost::asio::strand strand_;
	MapSnake snakes_;
	//ListApple apples_;

	std::default_random_engine random_engine_;

	// to make new snake...
	const double radius{ 5. };		// UNIT
	const double velocity{ 0.06 };	// UNIT/ms
	const Position2D player_pos{ 100, 100 };

	// my update event... pass zone event to listners
	SbGeoZone::UpdateEvent update_event_;	

	// zone event connection
	std::list<std::unique_ptr<typename SbGeoZone::UpdateEventRelayer>> list_zone_event_relayer_;
	SbGeoZoneGrid zone_grid_;

	//임시
	friend class LocalGameClient;
};

typedef std::shared_ptr<GameSession> GameSessionSP;
typedef std::weak_ptr<GameSession> GameSessionWP;

}
}
#endif //SNAKEBITE_GAME_SESSION_H_