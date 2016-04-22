#ifndef SNAKEBITE_GAME_GEO_ZONE_H_
#define SNAKEBITE_GAME_GEO_ZONE_H_

#include <SDKDDKVer.h>
#include <boost/asio.hpp>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <string>

#include "game_geo_zone_define.h"

namespace acoross {
namespace snakebite {

//////////////////////////////////////////////////
// 단일 게임을 여러개의 zone 으로 쪼개어 쓴다.
class GameGeoZoneGrid;

// serialized by strand
class GameGeoZone
	: public std::enable_shared_from_this<GameGeoZone>
{
public:
	using CloneSnakelistT = std::list<std::pair<Handle<Snake>::Type, ZoneObjectClone>>;
	using SharedCloneSnakelistT = std::shared_ptr<CloneSnakelistT>;
	using CloneSnakelistCallback = std::function<void(SharedCloneSnakelistT)>;
	using CloneApplelistT = std::list<ZoneObjectClone>;
	using SharedCloneApplelistT = std::shared_ptr<CloneApplelistT>;
	using CloneApplelistCallback = std::function<void(SharedCloneApplelistT)>;

	using ObserverFuncT = std::function<void(int idx_zone_x, int idx_zone_y,
		GameGeoZone::SharedCloneSnakelistT snakes, GameGeoZone::SharedCloneApplelistT apples)>;

	explicit GameGeoZone(
		::boost::asio::io_service& io_service,
		GameGeoZoneGrid& owner_zone_grid,
		int idx_zone_x, int idx_zone_y, 
		MovingObjectContainer& game_boundary, 
		int left, int top, int width, int height);
	~GameGeoZone();

	void Run(int frame_tick);
	void AsyncAddObserver(std::string name, ObserverFuncT func);
	void AsyncAddStaticObj(std::shared_ptr<Apple> apple);
	void AsyncRemoveStaticObj(Apple* apple, std::function<void(bool result)> func);
	void AsyncAddMovObj(std::shared_ptr<Snake> snake);

	size_t AtomicStaticObjCount() const;
	size_t AtomicMovObjCount() const;

public:
	const int IDX_ZONE_X;
	const int IDX_ZONE_Y;

private:
	// 다른 존하고의 충돌체크
	void AsyncProcessCollisionTo(
		std::shared_ptr<MapSnake> shared_snakes);

	// update loop, using timer
	void do_update();
	// @use in serializer
	// update every snake position
	void update_movobj_position(int64_t diff_in_ms);
	// @use in serializer
	// process collision with neighbor zones
	void process_collision(GameGeoZoneGrid& neighbors_);
	// 벽에 충돌했는지 체크.
	// 벽에 충돌하면 튕겨나옴.
	void process_collision_to_wall(SnakeSP actor);
	// zone 내부의 objects 위치를 broadcast
	void invoke_update_event_to_observers();

private:
	::boost::asio::strand strand_;
	::boost::asio::deadline_timer zone_timer_;
	int zone_timer_tick_{ 10 };
	std::atomic<bool> is_running_{ false };
	std::unordered_map<std::string, ObserverFuncT> observer_map_;

	GameGeoZoneGrid& owner_zone_grid_;
	MovingObjectContainer zone_boundary_;
	MovingObjectContainer& game_boundary_;

	//std::unordered_map<uintptr_t, std::shared_ptr<ZoneObjectBase>> mov_objects_;
	//std::unordered_map<uintptr_t, std::shared_ptr<ZoneObjectBase>> static_objects_;
	MapSnake mov_objects_;
	ListApple static_objects_;
	std::atomic<int> cached_mov_object_cnt_{ 0 };
	std::atomic<int> cached_static_object_cnt_{ 0 };

	CollisionSet wall_collision_set_;
};
//

}
}
#endif //SNAKEBITE_GAME_GEO_ZONE_H_