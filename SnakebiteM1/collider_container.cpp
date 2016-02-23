#include "stdafx.h"
#include "collider_container.h"

#include "geo_types.h"
#include "collider.h"
#include "moving_object.h"

namespace acoross {
namespace snakebite {

void ColliderContainer::CheckCollisions()
{
	// clean up collision
	for (auto& col1 : collider_handles_)
	{
		if (auto handle1 = col1.second.lock())
		{
			handle1->GetCollider().Collided = false;
		}
	}

	for (auto& col1 : collider_handles_)
	{
		if (auto handle1 = col1.second.lock())
		{
			for (auto& col2 : collider_handles_)
			{
				if (auto handle2 = col2.second.lock())
				{
					if (handle1.get() == handle2.get())
						continue;

					ColliderContainer::ProcessCollsion(*handle1, *handle2);
				}
			}
		}
	}
}

//static
void ColliderContainer::ProcessCollsion(ColliderHandle& c1, ColliderHandle& c2)
{
	ProcessCollsion(c1.GetCollider(), c2.GetCollider());
}

//static
void ColliderContainer::ProcessCollsion(Collider& c1, Collider& c2)
{
	// 같은 Id 끼리는 무시.
	if (c1.GetId() == c2.GetId())
		return;

	Collider* col_arr[2];

	col_arr[0] = &c1;
	col_arr[1] = &c2;

	auto& m1 = c1.GetMovingObject();
	auto& m2 = c2.GetMovingObject();

	if (acoross::snakebite::IsCrashed(m1, m2))
	{
		for (auto col_ptr : col_arr)
		{
			col_ptr->Collided = true;

			//auto it = map_collision_found_.find(col_ptr);
			//if (it != map_collision_found_.end())
			//{
			//	// 이미 충돌되어 있음.
			//}
			//else
			//{
			//	// 충돌 발생
			//	map_collision_found_[col_ptr] = col_ptr->GetHandle();

			//	col_ptr->Collided = true;
			//}
		}
	}
	else
	{
		for (auto col_ptr : col_arr)
		{
			//auto it = map_collision_found_.find(col_ptr);
			//if (it != map_collision_found_.end())
			//{
			//	// 충돌이 해소 됨.
			//	map_collision_found_.erase(it);

			//	col_ptr->Collided = false;
			//}
			//else
			//{
			//	// 충돌하지 않음.
			//}
		}
	}
}

}
}