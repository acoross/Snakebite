#ifndef SNAKEBITE_GAME_CLIENT_BASE_H_
#define SNAKEBITE_GAME_CLIENT_BASE_H_

#include <acoross/snakebite/win/WinWrapper.h>

#include <list>
#include <atomic>
#include <mutex>
#include <memory>
#include <cmath>
#include <strsafe.h>

#include <acoross/snakebite/util.h>
#include <acoross/snakebite/moving_object_system/moving_object_system.h>
#include <acoross/snakebite/game_session/game_session_system.h>
#include <acoross/snakebite/zone_system/zone_system.h>

#ifdef min
#undef min
#endif

namespace acoross {
namespace snakebite {

class GameClientBase
{
public:
	virtual ~GameClientBase() {}

	void SetPlayerHandleZero()
	{
		player_handle_ = 0;
		idx_zone_player_x = -1;
		idx_zone_player_y = -1;
	}

	void SetObjectList(
		int idx_x, int idx_y,
		SbGeoZone::CloneZoneObjListT&& snake_clone_list,
		SbGeoZone::CloneZoneObjListT&& apple_clone_list)
	{
		std::lock_guard<std::mutex> lock(clone_list_mutex_);
		zone_snake_clone_list_[std::make_pair(idx_x, idx_y)] = std::move(snake_clone_list);
		zone_apple_clone_list_[std::make_pair(idx_x, idx_y)] = std::move(apple_clone_list);
		zone_clone_list_changed_[std::make_pair(idx_x, idx_y)].store(true);
	}

	void RetrieveObjectList(
		int idx_x, int idx_y,
		SbGeoZone::CloneZoneObjListT& snake_clone_list,
		SbGeoZone::CloneZoneObjListT& apple_clone_list)
	{
		std::lock_guard<std::mutex> lock(clone_list_mutex_);
		
		auto it = zone_clone_list_changed_.find(std::make_pair(idx_x, idx_y));
		if (it == zone_clone_list_changed_.end())
			it->second.store(false);

		auto it_zone_snakes = zone_snake_clone_list_.find(std::make_pair(idx_x, idx_y));
		if (it_zone_snakes != zone_snake_clone_list_.end())
		{
			snake_clone_list = it_zone_snakes->second;
		}

		auto it_zone_apples = zone_apple_clone_list_.find(std::make_pair(idx_x, idx_y));
		if (it_zone_apples != zone_apple_clone_list_.end())
		{
			apple_clone_list = it_zone_apples->second;
		}
	}

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

	void SetGridOn(bool on)
	{
		bGridOn_ = on;
	}

	virtual void Draw(Win::WDC& wdc, RECT& client_rect)
	{
		if (zone_info_.initialized.load() == false)
		{
			return;
		}

		double scale_ratio = scale_pcnt_ / 100.;

		// client_rect 와 scale, zone grid size 를 비교하여 몇칸의 zone 을 그릴 지 결정한다.
		auto draw_zone_cnt_x = int(client_rect.right / (zone_info_.w * scale_ratio)) + 1;
		auto draw_zone_cnt_y = int(client_rect.bottom / (zone_info_.h * scale_ratio)) + 1;

		draw_zone_cnt_x = std::min(draw_zone_cnt_x, zone_info_.limit_idx_x);
		draw_zone_cnt_y = std::min(draw_zone_cnt_y, zone_info_.limit_idx_y);

		acoross::Win::WDC memdc(::CreateCompatibleDC(wdc.Get()));
		static HBITMAP hbitmap = ::CreateCompatibleBitmap(wdc.Get(),
			zone_info_.w * zone_info_.limit_idx_x, zone_info_.h * zone_info_.limit_idx_y);
		HBITMAP oldbit = (HBITMAP)::SelectObject(memdc.Get(), hbitmap);

		/*memdc.Rectangle(0, 0,
			zone_info_.w * draw_zone_cnt_x,
			zone_info_.h * draw_zone_cnt_y);*/

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

	//
	virtual void RequestZoneInfo() = 0;
	virtual void InitPlayer() = 0;
	virtual void SetKeyDown(PlayerKey player_key) = 0;
	virtual void SetKeyUp(PlayerKey player_key) = 0;

	std::atomic<size_t> snake_count_{ 0 };
	std::atomic<size_t> apple_count_{ 0 };
	std::atomic<double> mean_draw_time_ms_{ 0 };

protected:

	void DrawGrid(Win::WDC& memdc, int idx_x, int idx_y)
	{
		if (bGridOn_ == false)
		{
			return;
		}

		if (zone_info_.initialized.load() == false)
		{
			return;
		}

		auto it = zone_clone_list_changed_.find(std::make_pair(idx_x, idx_y));
		if (it == zone_clone_list_changed_.end() || it->second.load() == false)
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
		if (it == zone_clone_list_changed_.end() || it->second.load() == false)
		{
			return;
		}
		
		// snake 와 apple 의 복제본 리스트를 받아온 뒤 화면에 그린다.
		// 락을 짧은 순간만 걸기 때문에 효과적이라고 생각한다.
		SbGeoZone::CloneZoneObjListT snake_pairs;
		SbGeoZone::CloneZoneObjListT apples;
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
			for (auto& pair : apples)
			{
				DrawMovingObject(memdc, pair.second.head_);
				DrawObjectZoneIdx(memdc, pair.second, 7);
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

	//@need GameSession::snakes_mutex_ locked
	static void DrawSnake(Win::WDC& wdc, ZoneObjectClone& snake)
	{
		DrawSnakeName(wdc, snake);
		DrawObjectZoneIdx(wdc, snake, 35);

		DrawMovingObject(wdc, snake.head_);
		for (auto& body : snake.body_list_)
		{
			DrawMovingObject(wdc, body);
		}
	}
	static void DrawMovingObject(Win::WDC& wdc, MovingObject& mo)
	{
		const int radius = (int)mo.GetRadius();
		const auto pos = mo.GetPosition();
		const int center_x = (int)pos.x;
		const int center_y = (int)pos.y;

		wdc.Ellipse(center_x - radius, center_y - radius,
			center_x + radius, center_y + radius);
	}
	static void DrawSnakeName(Win::WDC& wdc, ZoneObjectClone& snake)
	{
		auto pos = snake.head_.GetPosition();
		RECT rect{
			(long)pos.x - 50, (long)pos.y - 20,
			(long)pos.x + 50, (long)pos.y - 5
		};
		wdc.DrawTextA(snake.Name, rect, DT_CENTER);
	}
	static void DrawObjectZoneIdx(Win::WDC& wdc, ZoneObjectClone& obj, int top)
	{
		auto pos = obj.head_.GetPosition();
		RECT rect{
			(long)pos.x - 50, (long)pos.y - top,
			(long)pos.x + 50, (long)pos.y - top + 15
		};
		char buf[20]{ 0, };
		::StringCchPrintfA(buf, _countof(buf), "%d, %d", obj.zone_idx_x_, obj.zone_idx_y_);

		wdc.DrawTextA(buf, rect, DT_CENTER);
	}
	//

	void set_player_handle(uintptr_t handle)
	{
		player_handle_ = handle;
	}

protected:
	using CloneSnakeList = SbGeoZone::CloneZoneObjListT;
	using CloneAppleList = SbGeoZone::CloneZoneObjListT;
	std::mutex clone_list_mutex_;
	std::map<std::pair<int, int>, CloneSnakeList> zone_snake_clone_list_;
	std::map<std::pair<int, int>, CloneAppleList> zone_apple_clone_list_;
	std::map<std::pair<int, int>, std::atomic<bool>> zone_clone_list_changed_;

	struct ZoneInfo
	{
		std::atomic<bool> initialized{ false };
		int limit_idx_x{ 1 };
		int limit_idx_y{ 1 };
		int w{ 1 };
		int h{ 1 };
	} zone_info_;

	std::atomic<int> scale_pcnt_{ 100 };
	int idx_zone_player_x{ -1 };
	int idx_zone_player_y{ -1 };

	//FIXME: !!! player_ 의 exchange 가 atomic 해야함
	// lock 을 추가하던지 뭔가 코드 수정 필요.
	Handle<Snake>::Type player_handle_{ 0 };
	PlayerKey player_key_{ PlayerKey::PK_NONE };

	//
	bool bGridOn_{ true };
};

}
}
#endif //SNAKEBITE_GAME_CLIENT_BASE_H_