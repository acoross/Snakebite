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
	{
		RequestZoneInfo();
	}
	virtual ~LocalGameClient(){}
	
	virtual void Draw(Win::WDC& wdc, RECT& client_rect) override
	{
		if (zone_info_.initialized.load() == false)
		{
			return;
		}

		double scale_ratio = scale_pcnt_ / 100.;

		// client_rect 와 scale, zone grid size 를 비교하여 몇칸의 zone 을 그릴 지 결정한다.
		auto draw_zone_cnt_x = int(client_rect.right / (zone_info_.w * scale_ratio)) + 1;
		auto draw_zone_cnt_y = int(client_rect.bottom / (zone_info_.h * scale_ratio)) + 1;

		draw_zone_cnt_x = min(draw_zone_cnt_x, zone_info_.limit_idx_x);
		draw_zone_cnt_y = min(draw_zone_cnt_y, zone_info_.limit_idx_y);

		acoross::Win::WDC memdc(::CreateCompatibleDC(wdc.Get()));
		static HBITMAP hbitmap = ::CreateCompatibleBitmap(wdc.Get(), 
			zone_info_.w * zone_info_.limit_idx_x, zone_info_.h * zone_info_.limit_idx_y);
		HBITMAP oldbit = (HBITMAP)::SelectObject(memdc.Get(), hbitmap);
		
		for (int idx_x = 0; idx_x < draw_zone_cnt_x; ++idx_x)
		{
			for (int idx_y = 0; idx_y < draw_zone_cnt_y; ++idx_y)
			{
				DrawGrid(memdc, idx_x, idx_y);
			}
		}

		for (int idx_x = 0; idx_x < draw_zone_cnt_x; ++idx_x)
		{
			for (int idx_y = 0; idx_y < draw_zone_cnt_y; ++idx_y)
			{
				DrawZone(memdc, idx_x, idx_y);
			}
		}

		::StretchBlt(
			wdc.Get(), 
			0, 0, 
			int(scale_ratio * zone_info_.w * draw_zone_cnt_x),
			int(scale_ratio * zone_info_.h * draw_zone_cnt_y),
			memdc.Get(), 
			0, 0, 
			zone_info_.w * draw_zone_cnt_x, 
			zone_info_.h * draw_zone_cnt_y,
			SRCCOPY);
		//::BitBlt(wdc.Get(), 0, 0, client_rect.right, client_rect.bottom, memdc.Get(), 0, 0, SRCCOPY);

		::SelectObject(memdc.Get(), oldbit);
		::DeleteObject(memdc.Get());
	}

	void DrawGrid(Win::WDC& memdc, int idx_x, int idx_y)
	{
		if (zone_info_.initialized.load() == false)
		{
			return;
		}

		auto it = zone_clone_list_changed_.find(std::make_pair(idx_x, idx_y));
		if (it == zone_clone_list_changed_.end() || it->second == false)
		{
			return;
		}

		RECT zone_rect = { zone_info_.w * idx_x, zone_info_.h * idx_y,
			zone_info_.w * (idx_x + 1), zone_info_.h * (idx_y + 1) };

		// 테두리 그리기
		if (idx_zone_player_x == idx_x && idx_zone_player_y == idx_y)
		{
			HBRUSH oldbrush = (HBRUSH)::SelectObject(memdc.Get(), ::GetStockObject(GRAY_BRUSH));
			memdc.Rectangle(zone_rect.left, zone_rect.top, zone_rect.right, zone_rect.bottom);
			(HBRUSH)::SelectObject(memdc.Get(), oldbrush);
		}
		else
		{
			memdc.Rectangle(zone_rect.left, zone_rect.top, zone_rect.right, zone_rect.bottom);
		}

		{
			wchar_t buf[100]{ 0, };
			::StringCchPrintfW(buf, _countof(buf), L"%d, %d", idx_x, idx_y);
			auto orgcolor = SetTextColor(memdc.Get(), RGB(255, 0, 0));
			memdc.DrawTextW(buf, zone_rect, DT_CENTER);
			::SetTextColor(memdc.Get(), orgcolor);
		}
	}

	void DrawZone(Win::WDC& memdc, int idx_x, int idx_y)
	{
		if (zone_info_.initialized.load() == false)
		{
			return;
		}

		auto it = zone_clone_list_changed_.find(std::make_pair(idx_x, idx_y));
		if (it == zone_clone_list_changed_.end() || it->second == false)
		{
			return;
		}

		// snake 와 apple 의 복제본 리스트를 받아온 뒤 화면에 그린다.
		// 락을 짧은 순간만 걸기 때문에 효과적이라고 생각한다.
		std::list<std::pair<Handle<Snake>::Type, ZoneObjectClone>> snake_pairs;
		std::list<ZoneObjectClone> apples;
		RetrieveObjectList(idx_x, idx_y, snake_pairs, apples);
		//

		// TODO
		// 화면과 game_session 크기를 고려해 ratio 를 정한 뒤,
		// ratio 에 따라 크기를 조절해서 그린다.

		{
			MeanProcessTimeChecker mean_draw(mean_draw_time_ms_);

			
			HBRUSH oldbrush = (HBRUSH)::SelectObject(memdc.Get(), ::GetStockObject(DC_BRUSH));
			auto oldcol = ::SetDCBrushColor(memdc.Get(), RGB(0, 0, 255));
			auto oldtextcol = ::SetTextColor(memdc.Get(), RGB(0, 0, 255));
			for (auto& apple : apples)
			{
				DrawMovingObject(memdc, apple.head_);
				DrawObjectZoneIdx(memdc, apple, 7);
			}
			::SetTextColor(memdc.Get(), oldtextcol);
			::SelectObject(memdc.Get(), oldbrush);

			for (auto& snake_pair : snake_pairs)
			{
				if (snake_pair.first == player_handle_)
				{
					idx_zone_player_x = snake_pair.second.zone_idx_x_;
					idx_zone_player_y = snake_pair.second.zone_idx_y_;

					HBRUSH oldbrush = (HBRUSH)::SelectObject(memdc.Get(), ::GetStockObject(BLACK_BRUSH));
					DrawSnake(memdc, snake_pair.second);
					(HBRUSH)::SelectObject(memdc.Get(), oldbrush);
				}
				else
				{
					DrawSnake(memdc, snake_pair.second);
				}
			}
		}
	}

	void RequestZoneInfo()
	{
		zone_info_.limit_idx_x = game_server_.COUNT_ZONE_X;
		zone_info_.limit_idx_y = game_server_.COUNT_ZONE_Y;
		zone_info_.w = game_server_.ZoneWidth;
		zone_info_.h = game_server_.ZoneHeight;
		zone_info_.initialized.store(true);
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
	
	int FetchAddScalePcnt(int val)
	{
		auto ret = scale_pcnt_.fetch_add(val);
		if (ret < 0)
		{
			return scale_pcnt_.compare_exchange_strong(ret, 0);
		}
		if (ret >= 100)
		{
			return scale_pcnt_.compare_exchange_strong(ret, 100);
		}
		return ret;
	}
private:
	std::atomic<int> scale_pcnt_{ 100 };
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