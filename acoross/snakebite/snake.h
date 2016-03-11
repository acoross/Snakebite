#ifndef SNAKEBITE_SNAKE_PEICE_H_
#define SNAKEBITE_SNAKE_PEICE_H_

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include <acoross/util.h>
#include "snake_collider.h"

namespace acoross {
namespace snakebite {

class Snake
{
public:
	using MyMovingObject = MovingObject<ColliderBase>;
	using MyContainer = MovingObjectContainer<MyMovingObject>;

	Snake(Snake&) = delete;
	Snake& operator=(Snake&) = delete;

	Snake(MyContainer& container
		, const Position2D& pos, double radius, const Degree& angle, double velocity, double ang_vel, int len)
		: container_(container), angle_(angle), velocity_(velocity), ang_vel_(ang_vel)
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
	~Snake() 
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

	void OnCollideStart(std::shared_ptr<Snake> other)
	{
		//this->Turn(180);
		ProcDie();
	}
	void OnColliding(std::shared_ptr<Snake> other)
	{

	}
	void OnCollideEnd(std::shared_ptr<Snake> other)
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
	MyContainer& container_;
};

inline void Snake::Move(const DirVector2D & diff_vec)
{
	Position2D pos_prev = head_->GetPosition();
	DirVector2D diff_prev = diff_vec;

	head_->Move(diff_vec);

	for (auto mo : body_list_)
	{
		Position2D pos_now = mo->GetPosition();
		DirVector2D diff = { pos_prev.x - pos_now.x, pos_prev.y - pos_now.y };

		double limitdist = 2 * (mo->GetRadius()) * 0.9;
		double piece_dist = diff.Length();

		if (piece_dist >= limitdist)
		{
			double diff_len = diff_prev.Length();

			diff = diff.GetNormalized();
			diff.x *= diff_len;
			diff.y *= diff_len;

			pos_prev = mo->GetPosition();
			mo->Move(diff);

			diff_prev = diff;
		}
		else
		{
			break;
		}
	}
}


}
}

#endif //SNAKEBITE_SNAKE_PEICE_H_