#include "moving_object.h"

namespace acoross {
namespace snakebite {

bool IsCrashed(const MovingObject & mo1, const MovingObject & mo2)
{
	double dist = Position2D::Distance(mo1.GetPosition(), mo2.GetPosition());

	if (dist < mo1.GetRadius() + mo2.GetRadius())
	{
		return true;
	}

	return false;
}

void MovingObject::Move(const DirVector2D & diff)
{
	// 테두리 밖으로 벗어나지 않도록 막음.
	auto pos_new = pos_;
	pos_new.x += diff.x;
	pos_new.y += diff.y;

	if (pos_new.x > container_.Left && pos_new.x < container_.Right)
	{
		pos_.x = pos_new.x;
	}

	if (pos_new.y > container_.Top && pos_new.y < container_.Bottom)
	{
		pos_.y = pos_new.y;
	}
}

////static
//void MovingObject::ProcessCollsion(MovingObject & m1, MovingObject & m2)
//{
//	// 같은 Id 끼리는 무시.
//	if (m1.GetId() == m2.GetId())
//		return;
//
//	if (acoross::snakebite::IsCrashed(m1, m2))
//	{
//		m1.Collided = true;
//		m2.Collided = true;
//
//		if (m1.collideCallback_)
//			m1.collideCallback_(m2);
//		if (m2.collideCallback_)
//			m2.collideCallback_(m1);
//	}
//}

}
}
