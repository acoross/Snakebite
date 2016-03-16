#ifndef SNAKEBITE_CONTAINER_DRAWER_H_
#define SNAKEBITE_CONTAINER_DRAWER_H_

#include <acoross/snakebite/win/WinWrapper.h>
#include <memory>
#include <mutex>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include <acoross/snakebite/game_session.h>
#include <acoross/snakebite/snake.h>

namespace acoross {
namespace snakebite {

class GameClient final
{
public:
	GameClient(GameSession& game_session)
		: game_session_(game_session)
	{}
	virtual ~GameClient(){}

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
		
		{
			std::lock_guard<std::recursive_mutex> lock(game_session_.LockSnakes());

			auto snakes = game_session_.snakes_;
			auto player = player_.lock();
			for (auto pair : snakes)
			{
				auto& snake = pair.second;

				if (snake == player_.lock())
				{
					HBRUSH oldbrush = (HBRUSH)::SelectObject(wdc.Get(), ::GetStockObject(BLACK_BRUSH));
					DrawSnake(wdc, *player);
					(HBRUSH)::SelectObject(wdc.Get(), oldbrush);
				}
				else
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
	}

	void InitPlayer()
	{
		if (auto player = player_.lock())
		{
			game_session_.RemoveSnake(player.get());
		}

		player_ = game_session_.AddSnake();
	}

	void SetKeyDown(PlayerKey player_key)
	{
		if (auto player = player_.lock())
		{
			player->SetPlayerKey(player_key);
		}
	}

	void SetKeyUp(PlayerKey player_key)
	{
		if (auto player = player_.lock())
		{
			player->SetKeyUp(player_key);
		}
	}

private:
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

	std::weak_ptr<Snake> player_;
	GameSession& game_session_;
};

}
}
#endif //SNAKEBITE_CONTAINER_DRAWER_H_