#ifndef SNAKEBITE_CONTAINER_DRAWER_H_
#define SNAKEBITE_CONTAINER_DRAWER_H_

#include <acoross/snakebite/win/WinWrapper.h>
#include <acoross/snakebite/moving_object_system/moving_object_system.h>

namespace acoross {
namespace snakebite {

class ContainerDrawer final
{
public:
	ContainerDrawer(MovingObjectContainer& container)
		: container_(container)
	{}
	virtual ~ContainerDrawer(){}

	void Draw(Win::WDC& wdc)
	{
		double ratio = 1.0;

		// 테두리 그리기
		wdc.Rectangle(container_.Left, container_.Top,
			container_.Right, container_.Bottom);
			
		// TODO
		// 화면과 game_session 크기를 고려해 ratio 를 정한 뒤,
		// ratio 에 따라 크기를 조절해서 그린다.
			
		auto& mo_list = container_.GetMovingObjects();
		for (auto& mo : mo_list)
		{
			const int radius = (int)mo->GetRadius();

			auto pos = mo->GetPosition();
			//auto ang = mo->GetAngle();

			int center_x = (int)pos.x;
			int center_y = (int)pos.y;

			/*if (mo->Collided)
			{
				HBRUSH oldbrush = (HBRUSH)::SelectObject(wdc.Get(), ::GetStockObject(BLACK_BRUSH));
					
				wdc.Ellipse(center_x - radius, center_y - radius,
					center_x + radius, center_y + radius);

				(HBRUSH)::SelectObject(wdc.Get(), oldbrush);
			}
			else*/
			{
				/*if (mo->GetId() == 0)
				{
					HBRUSH oldbrush = (HBRUSH)::SelectObject(wdc.Get(), ::GetStockObject(LTGRAY_BRUSH));
					wdc.Ellipse(center_x - radius, center_y - radius,
						center_x + radius, center_y + radius);
					(HBRUSH)::SelectObject(wdc.Get(), oldbrush);
				}
				else*/
				{
					wdc.Ellipse(center_x - radius, center_y - radius,
						center_x + radius, center_y + radius);
				}
			}
		}
	}

private:
	MovingObjectContainer& container_;
};

}
}
#endif //SNAKEBITE_CONTAINER_DRAWER_H_