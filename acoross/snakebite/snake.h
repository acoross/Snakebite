#ifndef SNAKEBITE_SNAKE_PEICE_H_
#define SNAKEBITE_SNAKE_PEICE_H_

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include <acoross/util.h>
#include "game_object.h"
#include "game_session.h"

namespace acoross {
namespace snakebite {

class Apple;
class GameSession;

class Snake : public GameObject
{
public:
	using CollisionSet = std::set<GameObject*>;

	Snake(Snake&) = delete;
	Snake& operator=(Snake&) = delete;

	Snake(GameSession& game_session, MyContainer& container, const Position2D& pos, double radius
		, const Degree& angle, double velocity, double ang_vel, int len);
	virtual ~Snake();
	
	void Move(const DirVector2D& diff_vec);

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
	Position2D GetPosition() const { return head_->GetPosition(); }

	void AddBody();

	bool ProcessCollision(std::shared_ptr<GameObject> target);

public:
	GameSession& game_session_;

private:
	Degree angle_; // degree
	double velocity_; // UNIT/ms
	double ang_vel_;	// degree/ms

	CollisionSet collision_set_;
};

}
}

#endif //SNAKEBITE_SNAKE_PEICE_H_