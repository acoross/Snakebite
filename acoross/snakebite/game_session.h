#ifndef SNAKEBITE_GAME_SESSION_H_
#define SNAKEBITE_GAME_SESSION_H_

#include <list>
#include <map>
#include <memory>
#include <utility>
#include <random>
#include <mutex>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "snake.h"
#include "Apple.h"

namespace acoross {
namespace snakebite {

// 맵, MovingObject 로 구성되는 하나의 게임 단위.
class GameSession final
{
public:
	using MyMovingObject = MovingObject;
	using MyContainer = MovingObjectContainer;
	using ListMovingObject = MyContainer::ListMovingObject;

	//using ListSnake = std::list<std::shared_ptr<Snake>>;
	//using ListSnakeNpc = std::list<std::weak_ptr<Snake>>;
	using MapSnake = std::map<Snake*, std::shared_ptr<Snake>>;
	using MapSnakeWP = std::map<Snake*, std::weak_ptr<Snake>>;
	using ListApple = std::list<std::shared_ptr<Apple>>;
	using ListGameObject = std::list<std::shared_ptr<GameObject>>;

	explicit GameSession(unsigned int init_snake_count = 1, unsigned int init_apple_count = 20);
	~GameSession();

	void UpdateMove(int64_t diff_in_ms);
	void ProcessCollisions();

	//임시로 열어주는 API
	ListMovingObject& GetMovingObjects() { return container_.GetMovingObjects(); }

	/* 플레이어 조작은 Snake 를 직접 조작
	void SetPlayerKey(PlayerKey player_key) { last_pk_ = player_key; }
	void SetKeyUp(PlayerKey player_key)
	{
		if (last_pk_ == player_key)
		{
			last_pk_ = PK_NONE;
		}
	}

	PlayerKey GetPlayerKey() const { return last_pk_; }
	PlayerKey RetrievePlayerKey()
	{
		auto ret = last_pk_;
		last_pk_ = PK_NONE;
		return ret;
	}
	*/

	MyContainer& GetContainer() { return container_; }

	bool RemoveSnake(Snake* snake);
	bool RemoveApple(Apple* apple);

	std::shared_ptr<Snake> AddSnake();
	void AddApple();
	void InitPlayer();

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

	std::recursive_mutex& LockSnakes()
	{
		return snakes_mutex_;
	}

	std::shared_ptr<Snake> GetPlayer()
	{
		return player_.lock();
	}

private:
	using GameObjectWP = std::weak_ptr<GameObject>;
	using SnakeWP = std::weak_ptr<Snake>;
	using CollisionMap = std::map<Snake*, GameObjectWP>;
	using CollisionSet = std::set<Snake*>;

	//void ProcessCollision(std::shared_ptr<Snake> actor, std::shared_ptr<Snake> target);
	//void ProcessCollisionToApple(std::shared_ptr<Snake> actor, std::shared_ptr<Apple> target);
	void ProcessCollisionToWall(std::shared_ptr<Snake> actor);

	std::default_random_engine random_engine_;
	CollisionSet wall_collision_set_;
	CollisionMap collision_map_;
	MyContainer container_;
	
#pragma region snakes
	std::weak_ptr<Snake> player_;
	MapSnakeWP snake_npcs_;
	MapSnake snakes_;
	ListApple apples_;
	PlayerKey last_pk_{ PK_NONE };

	// 병신같지만 drawer 랑 동기화 하기 위해 추가한 lock 이다...
	// 수정 필요.
	std::recursive_mutex snakes_mutex_;
#pragma endregion snakes

	const double radius{ 5. };		// UNIT
	const double velocity{ 0.06 };	// UNIT/ms
	const Position2D player_pos{ 100, 100 };

	friend class GameSessionDrawer;
};

typedef std::shared_ptr<GameSession> GameSessionSP;
typedef std::weak_ptr<GameSession> GameSessionWP;

}
}
#endif //SNAKEBITE_GAME_SESSION_H_