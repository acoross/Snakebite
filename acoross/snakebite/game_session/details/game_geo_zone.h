#ifndef SNAKEBITE_GAME_GEO_ZONE_H_
#define SNAKEBITE_GAME_GEO_ZONE_H_

#include <SDKDDKVer.h>
#include <boost/asio.hpp>
#include <mutex>
#include <vector>
#include <queue>

#include "game_geo_zone_define.h"

namespace acoross {
namespace snakebite {

//////////////////////////////////////////////////
// 단일 게임을 여러개의 zone 으로 쪼개어 쓴다.
class GameGeoZoneGrid;

class ZoneObjectBase
{
public:

};

template <typename T>
class ZoneObject : ZoneObjectBase
{

};

// serialized by strand
class GameGeoZone
	: public std::enable_shared_from_this<GameGeoZone>
{
public:
	using CloneSnakelistT = std::list<std::pair<Handle<Snake>::Type, GameObjectClone>>;
	using SharedCloneSnakelistT = std::shared_ptr<CloneSnakelistT>;
	using CloneSnakelistCallback = std::function<void(SharedCloneSnakelistT)>;
	using CloneApplelistT = std::list<GameObjectClone>;
	using SharedCloneApplelistT = std::shared_ptr<CloneApplelistT>;
	using CloneApplelistCallback = std::function<void(SharedCloneApplelistT)>;

	explicit GameGeoZone(
		::boost::asio::io_service& io_service,
		int idx_zone_x, int idx_zone_y, 
		MovingObjectContainer& game_boundary, 
		int left, int top, int width, int height);
	~GameGeoZone();

	// update every snake position
	void AsyncUpdateMove(int64_t diff_in_ms);
	// process collision with neighbor zones
	void AsyncProcessCollision(GameGeoZoneGrid& neighbors_);

	void AsyncAddStaticObj(std::shared_ptr<Apple> apple);
	void AsyncRemoveStaticObj(Apple* apple, std::function<void(bool result)> func);
	void AsyncAddMovObj(std::shared_ptr<Snake> snake);
	void AsyncRemoveMovObj(Handle<Snake>::Type snake);

	size_t AtomicStaticObjCount() const;
	size_t AtomicMovObjCount() const;

	void AsyncCloneMovObjList(CloneSnakelistCallback func);
	void AsyncCloneStaticObjList(CloneApplelistCallback func);

public:
	const int IDX_ZONE_X;
	const int IDX_ZONE_Y;

private:
	// 다른 존하고의 충돌체크
	void async_process_collision_to(
		std::shared_ptr<MapSnake> shared_snakes);

	// 벽에 충돌했는지 체크.
	// 벽에 충돌하면 튕겨나옴.
	void process_collision_to_wall(SnakeSP actor);

private:
	::boost::asio::strand strand_;

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


class SnakebiteGeoZone
{

};

}
}
#endif //SNAKEBITE_GAME_GEO_ZONE_H_