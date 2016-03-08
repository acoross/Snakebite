#ifndef SNAKEBITE_SNAKE_PEICE_H_
#define SNAKEBITE_SNAKE_PEICE_H_

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include <acoross/util.h>

namespace acoross {
namespace snakebite {

// reference type
class SnakePiece : public MovingObject
{
public:
	SnakePiece(SnakePiece&) = delete;
	SnakePiece& operator=(SnakePiece&) = delete;

	SnakePiece(MovingObjectContainer& container, int Id, const Position2D& pos, double radius, const Degree& angle, double velocity, double ang_vel)
		: MovingObject(container, Id, pos, radius)
		, angle_(angle), velocity_(velocity), ang_vel_(ang_vel)
	{}

	virtual ~SnakePiece() {}

	void SetAngle(const Degree& angle)
	{
		angle_ = angle;
	}

	virtual void Move(const DirVector2D& diff_vec) override;

	void Turn(const Degree& diff)
	{
		angle_ = angle_ + diff;
	}

	void AddToTail(std::weak_ptr<SnakePiece> snake_new_wp)
	{
		if (auto next = snake_body_next_.lock())
		{
			if (auto snake_new = snake_new_wp.lock())
			{
				snake_new->MoveTo(next->GetPosition());
				next->AddToTail(snake_new_wp);
			}
		}
		else
		{
			snake_body_next_ = snake_new_wp;
		}
	}

	Degree GetAngle() const { return angle_; }
	double GetVelocity() const { return velocity_; }
	double GetAngVelocity() const { return ang_vel_; }

private:
	Degree angle_; // degree
	double velocity_; // UNIT/ms
	double ang_vel_;	// degree/ms

	std::weak_ptr<SnakePiece> snake_body_next_;
};

}
}

#endif //SNAKEBITE_SNAKE_PEICE_H_