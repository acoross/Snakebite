#ifndef SNAKEBITE_SNAKE_PEICE_H_
#define SNAKEBITE_SNAKE_PEICE_H_

#include <atomic>
#include <functional>
#include <string>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include "sb_zone_object.h"

namespace acoross {
namespace snakebite {

enum PlayerKey
{
	PK_NONE,
	PK_RIGHT,
	PK_LEFT,
	PK_MAX
};

class GameSession;

class Snake : public SbZoneObject
{
public:
	using EventHandler = std::function<void(Snake&)>;

	Snake(GameSession& game_session, const Position2D& pos, double radius
		, const Degree& angle, double velocity, double ang_vel, int len
		, EventHandler onDie = EventHandler(), std::string name = "noname");
	virtual ~Snake();

	virtual void UpdateMove(int64_t diff_in_ms, MovingObjectContainer& container) override;

	void Move(const DirVector2D& diff_vec, MovingObjectContainer& container);

	//@thread-safe: atomic once
	void Die();

	void SetAngle(const Degree& angle)
	{
		angle_ = angle;
	}
	void Turn(const Degree& diff)
	{
		angle_ = angle_ + diff;
	}
	Degree GetAngle() const { return angle_; }
	double GetVelocity() const { return velocity_; }
	double GetAngVelocity() const { return ang_vel_; }

#pragma region key_input
	//@atomic
	void SetKeyDown(PlayerKey player_key) 
	{ 
		last_pk_.store(player_key);
	}
	void SetKeyUp(PlayerKey player_key)
	{
		last_pk_.compare_exchange_strong(player_key, PK_NONE);
	}
	PlayerKey GetPlayerKey() const { return last_pk_.load(); }
	//
#pragma endregion key_input

public:
	GameSession& game_session_;

private:
	Degree angle_; // degree
	double velocity_; // UNIT/ms
	double ang_vel_;	// degree/ms

	std::atomic<PlayerKey> last_pk_{ PK_NONE };

	std::atomic<bool> isAlive_{ true };
	EventHandler onDie_;
};
using SnakeSP = std::shared_ptr<Snake>;
using SnakeWP = std::weak_ptr<Snake>;

class SnakeHandle final
{
public:
	SnakeHandle()
		: handle_(0)
	{}

	SnakeHandle(SnakeSP snake)
		: handle_(reinterpret_cast<uintptr_t>(snake.get()))
		, snake_wp_(snake)
	{}

	uintptr_t handle() const { return handle_; }
	const SnakeSP get() const
	{
		if (const auto snake = snake_wp_.lock())
		{
			return snake;
		}
		return SnakeSP();
	}

	void SetKeyDown(PlayerKey player_key)
	{
		if (auto snake = snake_wp_.lock())
		{
			snake->SetKeyDown(player_key);
		}
	}
	void SetKeyUp(PlayerKey player_key)
	{
		if (auto snake = snake_wp_.lock())
		{
			snake->SetKeyUp(player_key);
		}
	}

private:
	uintptr_t handle_;
	SnakeWP snake_wp_;
};

}
}

#endif //SNAKEBITE_SNAKE_PEICE_H_