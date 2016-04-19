#ifndef SNAKEBITE_GAME_GEO_ZONE_H_
#define SNAKEBITE_GAME_GEO_ZONE_H_

#include <mutex>
#include <vector>
#include <queue>
#include <boost/asio.hpp>

#include "game_geo_zone_define.h"

namespace acoross {
namespace snakebite {

//////////////////////////////////////////////////
// 단일 게임을 여러개의 zone 으로 쪼개어 쓴다.
class GameGeoZoneGrid;

// 하나의 GameZone 는 serializer 이용해 싱글스레드로 동작하게 해보자.
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

#pragma region use_snakes_mutex_
	// update every snake position
	void UpdateMove(int64_t diff_in_ms);
	// process collision with neighbor zones
	void ProcessCollision(GameGeoZoneGrid& neighbors_);

	void AddApple(std::shared_ptr<Apple> apple);
	void RemoveApple(Apple* apple);
	void AddSnake(std::shared_ptr<Snake> snake);
	void RemoveSnake(Handle<Snake>::Type snake);

	size_t AppleCount() const;
	size_t SnakeCount() const;

	void AsyncCloneSnakeList(CloneSnakelistCallback func);
	void AsyncCloneAppleList(CloneApplelistCallback func);
#pragma endregion use_snakes_mutex_

	/*void CheckAndRequestChangeZone();

	void RequestEnterZone(std::shared_ptr<Snake> snake, int org_idx_zone_x, int org_idx_zone_y);
	void ProcessAllEnterZoneRequests();

	void RequestLeaveZone(std::shared_ptr<Snake> snake, int org_idx_zone_x, int org_idx_zone_y);
	void ProcessAllLeaaveRequests();*/

public:
	const int IDX_ZONE_X;
	const int IDX_ZONE_Y;

private:
	// 다른 존하고의 충돌체크
	void ProcessCollisionTo(
		std::shared_ptr<MapSnake> shared_snakes);

	// 벽에 충돌했는지 체크.
	// 벽에 충돌하면 튕겨나옴.
	void process_collision_to_wall(SnakeSP actor);

	void shared_post(std::function<void()> func)
	{
		auto self(shared_from_this());
		strand_.post([func, self]()
		{
			func();
		});
	}
private:
	::boost::asio::strand strand_;

	MovingObjectContainer zone_boundary_;
	MovingObjectContainer& game_boundary_;

#pragma region snakes - use_snakes_mutex_
	MapSnake snakes_;
	ListApple apples_;
	std::atomic<int> cached_snake_cnt_{ 0 };
	std::atomic<int> cached_apple_cnt_{ 0 };
#pragma endregion snakes - use_snakes_mutex_

	CollisionSet wall_collision_set_;

	/*struct EnteringInfo
	{
		std::shared_ptr<Snake> snake;
		int org_idx_zone_x;
		int org_idx_zone_y;
	};
	std::recursive_mutex enter_leave_mutex_;
	std::queue<EnteringInfo> leaving_snakes_;
	std::queue<EnteringInfo> entering_snakes_;*/
};
//


}
}
#endif //SNAKEBITE_GAME_GEO_ZONE_H_