#ifndef SNAKEBITE_GAME_SESSION_H_
#define SNAKEBITE_GAME_SESSION_H_

#include <list>
#include <memory>
#include <utility>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "snake.h"

enum PlayerKey
{
	PK_NONE,
	PK_RIGHT,
	PK_LEFT
};

namespace acoross {
namespace snakebite {

// 맵, MovingObject 로 구성되는 하나의 게임 단위.
class GameSession final
{
public:
	using MyMovingObject = MovingObject<ColliderBase>;
	using MyContainer = MovingObjectContainer<MyMovingObject>;
	typedef MyContainer::ListMovingObject ListMovingObject;

	typedef std::list<std::shared_ptr<Snake>> ListSnake;

	GameSession() 
	{
		Initialize();
	}
	~GameSession() 
	{
		CleanUp();
	}

	void Initialize();
	void CleanUp();

	// moving_objects_ 의 위치를 갱신한다.
	void UpdateMove(int64_t diff_in_ms);
	
	void ProcessCollisions()
	{
		container_.ProcessCollisions();
	}

	//임시로 열어주는 API
	ListMovingObject& GetMovingObjects()
	{ 
		return container_.GetMovingObjects();
	}

	void SetPlayerKey(PlayerKey player_key)
	{
		last_pk_ = player_key;
	}

	void SetKeyUp(PlayerKey player_key)
	{
		if (last_pk_ == player_key)
		{
			last_pk_ = PK_NONE;
		}
	}

	PlayerKey GetPlayerKey() const
	{
		return last_pk_;
	}

	PlayerKey RetrievePlayerKey()
	{
		auto ret = last_pk_;
		last_pk_ = PK_NONE;
		return ret;
	}

	MyContainer& GetContainer() { return container_; }

private:
	MyContainer container_;
	std::shared_ptr<Snake> player_;
	ListSnake snakes_;
	PlayerKey last_pk_{ PK_NONE };
};

typedef std::shared_ptr<GameSession> GameSessionSP;
typedef std::weak_ptr<GameSession> GameSessionWP;

}
}
#endif //SNAKEBITE_GAME_SESSION_H_