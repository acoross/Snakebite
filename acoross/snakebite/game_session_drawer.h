#ifndef SNAKEBITE_CONTAINER_DRAWER_H_
#define SNAKEBITE_CONTAINER_DRAWER_H_

#include <acoross/snakebite/win/WinWrapper.h>
#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include <acoross/snakebite/game_session.h>

namespace acoross {
namespace snakebite {

class GameSessionDrawer final
{
public:
	GameSessionDrawer(GameSession& game_session)
		: game_session_(game_session)
	{}
	virtual ~GameSessionDrawer(){}

	void Draw(Win::WDC& wdc)
	{
		double ratio = 1.0;

		// 테두리 그리기
		auto& cont = game_session_.GetContainer();
		wdc.Rectangle(cont.Left, cont.Top,
			cont.Right, cont.Bottom);
		
		// TODO
		// 화면과 game_session 크기를 고려해 ratio 를 정한 뒤,
		// ratio 에 따라 크기를 조절해서 그린다.
		
		if (auto player = game_session_.player_.lock())
		{
			HBRUSH oldbrush = (HBRUSH)::SelectObject(wdc.Get(), ::GetStockObject(BLACK_BRUSH));
			DrawSnake(wdc, *player);
			(HBRUSH)::SelectObject(wdc.Get(), oldbrush);
		}

		auto snake_npc_list = game_session_.snake_npcs_;
		for (auto& snake_wp : snake_npc_list)
		{
			if (auto snake = snake_wp.lock())
			{
				DrawSnake(wdc, *snake);
			}
		}

		auto apple_list = game_session_.apples_;
		for (auto& apple : apple_list)
		{
			DrawMovingObject(wdc, *apple->head_);
		}
	}

	void DrawSnake(Win::WDC& wdc, Snake& snake)
	{
		DrawMovingObject(wdc, *snake.head_);
		for (auto& body : snake.body_list_)
		{
			DrawMovingObject(wdc, *body);
		}
	}

	void DrawMovingObject(Win::WDC& wdc, MovingObject& mo)
	{
		const int radius = (int)mo.GetRadius();
		const auto pos = mo.GetPosition();
		const int center_x = (int)pos.x;
		const int center_y = (int)pos.y;

		wdc.Ellipse(center_x - radius, center_y - radius,
			center_x + radius, center_y + radius);
	}

private:
	GameSession& game_session_;
};

}
}
#endif //SNAKEBITE_CONTAINER_DRAWER_H_