#ifndef SNAKEBITE_LOCAL_GAME_CLIENT_H_
#define SNAKEBITE_LOCAL_GAME_CLIENT_H_

#include <acoross/snakebite/win/WinWrapper.h>

#include <acoross/snakebite/game_session/game_session_system.h>
#include <acoross/snakebite/game_client_base.h>
#include "game_server.h"

namespace acoross {
namespace snakebite {

class LocalGameClient final
	: public GameClientBase
{
public:
	LocalGameClient(GameServer& game_server)
		: GameClientBase()
		, game_server_(game_server)
	{}
	virtual ~LocalGameClient(){}
	
	virtual void Draw(Win::WDC& wdc, RECT& client_rect) override
	{
		if (clone_list_changed_.load() == false)
		{
			return;
		}

		// snake 와 apple 의 복제본 리스트를 받아온 뒤 화면에 그린다.
		// 락을 짧은 순간만 걸기 때문에 효과적이라고 생각한다.
		std::list<std::pair<Handle<Snake>::Type, GameObjectClone>> snake_pairs;
		std::list<GameObjectClone> apples;
		RetrieveObjectList(snake_pairs, apples);
		//

		acoross::Win::WDC memdc(::CreateCompatibleDC(wdc.Get()));
		static HBITMAP hbitmap = ::CreateCompatibleBitmap(memdc.Get(), client_rect.right, client_rect.bottom);
		HBITMAP oldbit = (HBITMAP)::SelectObject(memdc.Get(), hbitmap);

		double ratio = 1.0;

		memdc.Rectangle(0, 0,
			game_server_.ZoneGridWidth, game_server_.ZoneGridHeight);

		// 테두리 그리기
		for (int i = 0; i < game_server_.COUNT_ZONE_X; ++i)
		{
			for (int j = 0; j < game_server_.COUNT_ZONE_Y; ++j)
			{
				if (idx_zone_player_x == i && idx_zone_player_y == j)
				{
					HBRUSH oldbrush = (HBRUSH)::SelectObject(memdc.Get(), ::GetStockObject(GRAY_BRUSH));
					memdc.Rectangle(
						game_server_.ZoneWidth * i,
						game_server_.ZoneHeight * j,
						game_server_.ZoneWidth * (i + 1),
						game_server_.ZoneHeight * (j + 1));
					(HBRUSH)::SelectObject(memdc.Get(), oldbrush);
				}
				else
				{
					memdc.Rectangle(
						game_server_.ZoneWidth * i,
						game_server_.ZoneHeight * j,
						game_server_.ZoneWidth * (i + 1),
						game_server_.ZoneHeight * (j + 1));
				}
			}
		}
		
		// TODO
		// 화면과 game_session 크기를 고려해 ratio 를 정한 뒤,
		// ratio 에 따라 크기를 조절해서 그린다.

		{
			MeanProcessTimeChecker mean_draw(mean_draw_time_ms_);
			for (auto& snake_pair : snake_pairs)
			{
				if (snake_pair.first == player_handle_)
				{
					auto& pos = snake_pair.second.body_list_.begin()->GetPosition();
					idx_zone_player_x = int(pos.x / game_server_.ZoneWidth);
					idx_zone_player_y = int(pos.y / game_server_.ZoneHeight);

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
	virtual void InitPlayer() override
	{
		game_server_.RequestToSession(
			[_this = this, handle = player_handle_](GameSession& session)
		{
			session.RemoveSnake(_this->player_handle_);
			_this->player_handle_ = session.MakeNewSnake("local player",
				[_this](Snake& snake)
			{
				_this->SetPlayerHandleZero();
			});
		});
	}

	void SetPlayerHandleZero()
	{
		player_handle_ = 0;
	}

	//

	//@atomic for Snake
	virtual void SetKeyDown(PlayerKey player_key) override
	{
		if (player_key == player_key_)
		{
			return;
		}

		player_key_ = player_key;

		game_server_.RequestToSession(
			[player_key, handle = player_handle_](GameSession& session)
		{
			session.RequestToSnake(handle, 
				[player_key](Snake& snake)
			{
				snake.SetKeyDown(player_key);
			});
		});
	}

	virtual void SetKeyUp(PlayerKey player_key) override
	{
		if (player_key != player_key_)
		{
			return;
		}

		player_key_ = PlayerKey::PK_NONE;

		game_server_.RequestToSession(
			[player_key, handle = player_handle_](GameSession& session)
		{
			session.RequestToSnake(handle,
				[player_key](Snake& snake)
			{
				snake.SetKeyUp(player_key);
			});
		});
	}
	//
	
private:
	int idx_zone_player_x{ -1 };
	int idx_zone_player_y{ -1 };

	//FIXME: !!! player_ 의 exchange 가 atomic 해야함
	// lock 을 추가하던지 뭔가 코드 수정 필요.
	Handle<Snake>::Type player_handle_{ 0 };
	PlayerKey player_key_{ PlayerKey::PK_NONE };
	GameServer& game_server_;
};

}
}
#endif //SNAKEBITE_LOCAL_GAME_CLIENT_H_