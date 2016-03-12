#ifndef SNAKEBITE_SNAKE_PEICE_H_
#define SNAKEBITE_SNAKE_PEICE_H_

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include <acoross/util.h>
#include "snake_collider.h"
#include "game_object.h"

namespace acoross {
namespace snakebite {

class Snake : public GameObject
{
public:
	Snake(Snake&) = delete;
	Snake& operator=(Snake&) = delete;

	Snake(MyContainer& container
		, const Position2D& pos, double radius, const Degree& angle, double velocity, double ang_vel, int len)
		: GameObject(container)
		, angle_(angle), velocity_(velocity), ang_vel_(ang_vel)
	{
		{
			auto mo = std::make_shared<MyMovingObject>(container, pos, radius, new PlayerHeadCollider(this));
			container.RegisterMovingObject(mo);
			head_ = mo;
		}
		
		for (int i = 0; i < len; ++i)
		{
			auto mo = std::make_shared<MyMovingObject>(container, pos, radius, new SnakeBodyCollider(this));
			container.RegisterMovingObject(mo);

			body_list_.push_back(mo);
		}
	}
	virtual ~Snake()
	{
		for (auto mo : body_list_)
		{
			container_.DeleteObject(mo);
		}
	}
	
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

	bool IsCollidingTo(std::shared_ptr<Snake> other) const
	{
		if (this == other.get())
		{
			return false;
		}

		if (IsCrashed(*head_, *other->head_))
		{
			return true;
		}

		for (auto mo : other->body_list_)
		{
			if (IsCrashed(*head_, *mo))
			{
				return true;
			}
		}

		return false;
	}

	void OnCollideStart(GameObject* other)
	{
		this->Turn(180);		
		//ProcDie();
	}
	void OnColliding(GameObject* other)
	{

	}
	void OnCollideEnd(GameObject* other)
	{

	}

	void ProcDie()
	{

	}

private:
	Degree angle_; // degree
	double velocity_; // UNIT/ms
	double ang_vel_;	// degree/ms

	std::shared_ptr<MyMovingObject> head_;
	std::list<std::shared_ptr<MyMovingObject>> body_list_;
};

inline void Snake::Move(const DirVector2D & diff_vec)
{
	const double dist_mov = diff_vec.Length();

	Position2D pos_prev_node = head_->GetPosition();
	head_->Move(diff_vec);

	//DirVector2D diff_prev = diff_vec;

	for (auto mo : body_list_)
	{
		const Position2D pos_current_node = mo->GetPosition();
		const DirVector2D diff_now_to_prev = { 
			pos_prev_node.x - pos_current_node.x, 
			pos_prev_node.y - pos_current_node.y 
		};
		
		const double limitdist = 2 * (mo->GetRadius()) * 0.9;
		if (diff_now_to_prev.Length() < limitdist)
		{
			break;
		}

		auto diff_move = diff_now_to_prev.GetNormalized();
		diff_move.x *= dist_mov;
		diff_move.y *= dist_mov;
		
		pos_prev_node = mo->GetPosition();
		mo->Move(diff_move);
	}
}

}
}

#endif //SNAKEBITE_SNAKE_PEICE_H_