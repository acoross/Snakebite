#ifndef SNAKEBITE_CONTAINER_DRAWER_H_
#define SNAKEBITE_CONTAINER_DRAWER_H_

#include <acoross/snakebite/win/WinWrapper.h>
#include <memory>
#include <mutex>
#include <atomic>

#include <acoross/snakebite/util.h>
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

	void SetObjectList(
		std::list<std::pair<Snake*, GameObjectClone>>&& snake_clone_list,
		std::list<GameObjectClone>&& apple_clone_list)
	{
		std::lock_guard<std::mutex> lock(clone_list_mutex_);
		snake_clone_list_ = std::move(snake_clone_list);
		apple_clone_list_ = std::move(apple_clone_list);
		clone_list_changed_.store(true);
	}

	void RetrieveObjectList(
		std::list<std::pair<Snake*, GameObjectClone>>& snake_clone_list,
		std::list<GameObjectClone>& apple_clone_list)
	{
		std::lock_guard<std::mutex> lock(clone_list_mutex_);
		snake_clone_list = std::move(snake_clone_list_);
		apple_clone_list = std::move(apple_clone_list_);
		clone_list_changed_.store(false);
	}

	void Draw(Win::WDC& wdc, RECT client_rect)
	{
		if (clone_list_changed_.load() == false)
		{
			return;
		}

		// snake 와 apple 의 복제본 리스트를 받아온 뒤 화면에 그린다.
		// 락을 짧은 순간만 걸기 때문에 효과적이라고 생각한다.
		std::list<std::pair<Snake*, GameObjectClone>> snake_pairs;
		std::list<GameObjectClone> apples;
		RetrieveObjectList(snake_pairs, apples);
		/*
		auto snake_pairs = game_session_.CloneSnakeList();
		auto apples = game_session_.CloneAppleList();
		*/
		//

		acoross::Win::WDC memdc(::CreateCompatibleDC(wdc.Get()));
		static HBITMAP hbitmap = ::CreateCompatibleBitmap(memdc.Get(), client_rect.right, client_rect.bottom);
		HBITMAP oldbit = (HBITMAP)::SelectObject(memdc.Get(), hbitmap);

		double ratio = 1.0;

		// 테두리 그리기
		auto& cont = game_session_.GetContainer();
		memdc.Rectangle(cont.Left, cont.Top,
			cont.Right, cont.Bottom);
		
		// TODO
		// 화면과 game_session 크기를 고려해 ratio 를 정한 뒤,
		// ratio 에 따라 크기를 조절해서 그린다.

		{
			MeanProcessTimeChecker mean_draw(mean_draw_time_ms_);
			auto player = player_.lock();
			for (auto& snake_pair : snake_pairs)
			{
				if (snake_pair.first == player.get())
				{
					HBRUSH oldbrush = (HBRUSH)::SelectObject(memdc.Get(), ::GetStockObject(BLACK_BRUSH));
					DrawSnake(memdc, snake_pair.second);
					(HBRUSH)::SelectObject(memdc.Get(), oldbrush);
				}
				else
				{
					DrawSnake(memdc, snake_pair.second);
				}
			}

			for (auto& apple : apples)
			{
				DrawMovingObject(memdc, apple.head_);
			}
		}

		::BitBlt(wdc.Get(), 0, 0, client_rect.right, client_rect.bottom, memdc.Get(), 0, 0, SRCCOPY);

		::SelectObject(memdc.Get(), oldbit);
		::DeleteObject(memdc.Get());
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

	std::atomic<double> mean_draw_time_ms_{ 0 };
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

	std::mutex clone_list_mutex_;
	std::list<std::pair<Snake*, GameObjectClone>> snake_clone_list_;
	std::list<GameObjectClone> apple_clone_list_;
	std::atomic<bool> clone_list_changed_{ false };
};

}
}
#endif //SNAKEBITE_CONTAINER_DRAWER_H_