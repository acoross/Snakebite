#ifndef SNAKEBITE_CONTAINER_DRAWER_H_
#define SNAKEBITE_CONTAINER_DRAWER_H_

#include <acoross/snakebite/win/WinWrapper.h>
#include <memory>
#include <mutex>

#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include <acoross/snakebite/game_session.h>
#include <acoross/snakebite/game_object.h>
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
			// snake 와 apple 의 복제본 리스트를 받아온 뒤 화면에 그린다.
			// 락을 짧은 순간만 걸기 때문에 효과적이라고 생각한다.
			// 복사생성은 비싸지 않은 작업이라고 생각했다.

			game_session_.LockSnakes().lock();
				auto snake_pairs = game_session_.CloneSnakeList();
				auto apples = game_session_.CloneAppleList();
			game_session_.LockSnakes().unlock();

			auto player = player_.lock();
			for (auto& snake_pair : snake_pairs)
			{
				if (snake_pair.first == player.get())
				{
					HBRUSH oldbrush = (HBRUSH)::SelectObject(wdc.Get(), ::GetStockObject(BLACK_BRUSH));
					DrawSnake(wdc, snake_pair.second);
					(HBRUSH)::SelectObject(wdc.Get(), oldbrush);
				}
				else
				{
					DrawSnake(wdc, snake_pair.second);
				}
			}
			
			for (auto& apple : apples)
			{
				DrawMovingObject(wdc, apple.head_);
			}
		}

		{
			/*std::lock_guard<std::recursive_mutex> lock(game_session_.LockSnakes());
			auto snakes = game_session_.snakes_;
			auto apples = game_session_.apples_;

			auto player = player_.lock();
			for (auto& pair : snakes)
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

			for (auto& apple : apples)
			{
				DrawMovingObject(wdc, apple->head_);
			}*/
		}
	}

	//@lock
	void InitPlayer()
	{
		if (auto player = player_.lock())
		{
			game_session_.RemoveSnake(player.get());
		}

		player_ = game_session_.AddSnake();
	}
	//

	//@atomic for Snake
	void SetKeyDown(PlayerKey player_key)
	{
		if (auto player = player_.lock())
		{
			player->SetKeyDown(player_key);
		}
	}

	void SetKeyUp(PlayerKey player_key)
	{
		if (auto player = player_.lock())
		{
			player->SetKeyUp(player_key);
		}
	}
	//

private:
	//@need GameSession::snakes_mutex_ locked
	void DrawSnake(Win::WDC& wdc, GameObjectClone& snake)
	{
		DrawMovingObject(wdc, snake.head_);
		for (auto& body : snake.body_list_)
		{
			DrawMovingObject(wdc, body);
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
	//

	std::weak_ptr<Snake> player_;
	GameSession& game_session_;
};

}
}
#endif //SNAKEBITE_CONTAINER_DRAWER_H_