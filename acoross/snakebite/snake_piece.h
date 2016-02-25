#ifndef SNAKEBITE_SNAKE_PEICE_H_
#define SNAKEBITE_SNAKE_PEICE_H_

#include <acoross/snakebite/moving_object_system/moving_object_system.h>

namespace acoross {
namespace snakebite {

class SnakePiece
{
public:
	template<typename... Args>
	SnakePiece(MovingObject& moving_object, const Degree& angle, double velocity, double ang_vel)
		: snake_body_next_(nullptr)
		, moving_object_(moving_object)
		, angle_(angle), velocity_(velocity), ang_vel_(ang_vel)
	{
		moving_object_.AddCollider();
	}

	~SnakePiece()
	{}

	void SetAngle(const Degree& angle)
	{
		angle_ = angle;
	}

	void Move(const DirVector2D& diff_vec);

	void Turn(const Degree& diff)
	{
		angle_ = angle_ + diff;
	}

	void AddToTail(SnakePiece* snake_new)
	{
		if (snake_body_next_.get() == nullptr)
			snake_body_next_.reset(snake_new);
		else
			snake_body_next_->AddToTail(snake_new);
	}

	Degree GetAngle() const { return angle_; }
	double GetVelocity() const { return velocity_; }
	double GetAngVelocity() const { return ang_vel_; }

	MovingObject& GetMovingObject() { return moving_object_; }

private:
	Degree angle_; // degree
	double velocity_; // UNIT/ms
	double ang_vel_;	// degree/ms

	std::unique_ptr<SnakePiece> snake_body_next_;
	MovingObject& moving_object_;
};

}
}

#endif //SNAKEBITE_SNAKE_PEICE_H_