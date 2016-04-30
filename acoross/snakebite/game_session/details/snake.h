#ifndef SNAKEBITE_SNAKE_PEICE_H_
#define SNAKEBITE_SNAKE_PEICE_H_

#include <atomic>
#include <functional>
#include <string>

#include <acoross/snakebite/zone_system/details/signal.h>
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

class SnakeNode
	: public SbZoneObject
{
public:
	template <typename T>
	using signal = boost::signals2::signal<T>;

	using UpdateEventListner = SbGeoZone::ObserverT;

	SnakeNode(GameSession& game_session, HandleT owner, SbColliderBase* collider, const Position2D& pos, double radius, std::string name);
	virtual ~SnakeNode();

	// snake 가 알아서 움직인다.
	virtual void UpdateMove(int64_t diff_in_ms, MovingObjectContainer& container) override = 0;

	// snake 를 지정한 diff_vec 만큼 이동시킨다.
	Position2D Move(const DirVector2D& diff_vec, MovingObjectContainer& container);

	//@lock
	void AddBody();
	//

	void SetLastPrevPosition(std::shared_ptr<Position2D> prev_last_pos)
	{
		std::atomic_store(&prev_last_pos_, prev_last_pos);
	}

public:
	GameSession& game_session_;
	const HandleT owner_handle_;

protected:
	const int max_internal_body_len_{ 5 };
	std::shared_ptr<SnakeNode> next_;
	std::shared_ptr<Position2D> prev_last_pos_;
};

///////////////////////////////////////////////////

class SnakeTail
	: public SnakeNode
{
public:
	SnakeTail(GameSession& game_session, HandleT owner, const Position2D& pos, double radius);
	virtual ~SnakeTail();

	// 앞 snake 를 따라서 이동한다.
	virtual void UpdateMove(int64_t diff_in_ms, MovingObjectContainer& container) override;
};

///////////////////////////////////////////////////
class Snake
	: public SnakeNode
{
public:
	using EventHandler = std::function<void(Snake&)>;

	Snake(GameSession& game_session, const Position2D& pos, double radius
		, const Degree& angle, double velocity, double ang_vel, int len
		, EventHandler onDie = EventHandler(), std::string name = "noname"
		, bool is_connect_to_zone = false);
	virtual ~Snake();

	virtual void UpdateMove(int64_t diff_in_ms, MovingObjectContainer& container) override;

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

	// reset_connection 내부의 connection.swap 이 thread-safe 하지 않을걸?
	// 그러므로, Enter 와 Leave 는 동시에 되지 않게 주의할 것.
	virtual void OnEnterZoneCallback(SbGeoZone& zone) override
	{
		if (is_connect_to_zone_)
		{
			std::atomic_exchange(&update_event_relayer_,
				std::shared_ptr<SbGeoZone::UpdateEventRelayer>(zone.GetUpdateEvent().make_relayer_up()));
		}
	}
	virtual void OnLeaveZoneCallback(SbGeoZone& zone) override
	{
		if (is_connect_to_zone_)
		{
			std::atomic_exchange(&update_event_relayer_, decltype(update_event_relayer_)());
		}
	}
	auto ConnectToUpdateEventRelayer(SbGeoZone::ObserverT on_update)
	{
		if (auto er = std::atomic_load(&update_event_relayer_))
		{
			return er->auto_connect(on_update);
		}

		return acoross::auto_connection();
	}

private:
	Degree angle_; // degree
	double velocity_; // UNIT/ms
	double ang_vel_;	// degree/ms

	std::atomic<PlayerKey> last_pk_{ PK_NONE };

	std::atomic<bool> isAlive_{ true };
	EventHandler onDie_;

	std::shared_ptr<SbGeoZone::UpdateEventRelayer> update_event_relayer_;
	const bool is_connect_to_zone_{ false };
};
using SnakeSP = std::shared_ptr<Snake>;
using SnakeWP = std::weak_ptr<Snake>;

}
}

#endif //SNAKEBITE_SNAKE_PEICE_H_