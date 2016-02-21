#include "stdafx.h"
#include "moving_object.h"

bool acoross::snakebite::IsCrashed(const MovingObject & mo1, const MovingObject & mo2)
{
	double dist = acoross::snakebite::CalcDistance(mo1.GetPosition(), mo2.GetPosition());

	if (dist < mo1.GetRadius() + mo2.GetRadius())
	{
		return true;
	}

	return false;
}

void acoross::snakebite::MovingObject::Move(const DirVector2D & diff)
{
	// TODO: [충돌했을 때, 겹쳐져 있는 상태, 충돌에서 벗어날 때] 를 구분해서 이벤트 발생시킨다.
	// 다른 오브젝트와 충돌했나?
	for (auto& mo2 : container_.GetMovingObjects())
	{
		if (this == mo2.get())
			continue;

		if (acoross::snakebite::IsCrashed(*this, *mo2))
		{
			//DoubleDispatch...
			std::cout << "crashed" << std::endl;
		}
	}

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
