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

#ifdef max
#undef max
#endif

namespace acoross {
namespace snakebite {

class GameClientBase
{
public:
	virtual ~GameClientBase() {}

	void SetPlayerPosition(int idx_x, int idx_y, int x, int y)
	{
		if (auto player_info = std::atomic_load(&player_info_))
		{
			player_info->SetPlayerPosition(idx_x, idx_y, x, y);
			if (follow_player_.load())
			{
				SetScreenCenterToPlayerPos();
			}
		}
	}

	void SetObjectList(
		int idx_x, int idx_y,
		SbGeoZone::CloneZoneObjListT&& snake_clone_list,
		SbGeoZone::CloneZoneObjListT&& apple_clone_list)
	{
		if (screen_on_.load() == false)
		{
			return;
		}

		std::lock_guard<std::mutex> lock(clone_list_mutex_);
		zone_snake_clone_list_[std::make_pair(idx_x, idx_y)] = std::move(snake_clone_list);
		zone_apple_clone_list_[std::make_pair(idx_x, idx_y)] = std::move(apple_clone_list);
		zone_clone_list_changed_[std::make_pair(idx_x, idx_y)].store(true);
	}

	void FetchMoveScreen(int x, int y)
	{
		center_screen_x_.fetch_add(x * 100 / scale_pcnt_);
		center_screen_y_.fetch_add(y * 100 / scale_pcnt_);
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

	void SetGridOnOff()
	{
		bGridOn_ = !bGridOn_;
	}

	void FollowPlayerOnOff()
	{
		follow_player_.store(!follow_player_.load());
	}

	void SetScreenCenterToPlayerPos()
	{
		if (auto player_info = std::atomic_load(&player_info_))
		{
			center_screen_x_.store(player_info->player_x_.load());
			center_screen_y_.store(player_info->player_y_.load());
		}
	}
	void SetScreenOnOff()
	{
		screen_on_.store(!screen_on_.load());
	}

	virtual void Draw(Win::WDC& wdc, RECT& client_rect)
	{
		if (zone_info_.initialized.load() == false)
		{
			return;
		}

		if (screen_on_.load() == false)
		{
			return;
		}

		acoross::Win::WDC memdc(::CreateCompatibleDC(wdc.Get()));
		static HBITMAP hbitmap = ::CreateCompatibleBitmap(wdc.Get(),
			zone_info_.w * zone_info_.limit_idx_x, zone_info_.h * zone_info_.limit_idx_y);
		HBITMAP oldbit = (HBITMAP)::SelectObject(memdc.Get(), hbitmap);

		/*memdc.Rectangle(0, 0,
			zone_info_.w * zone_info_.limit_idx_x,
			zone_info_.h * zone_info_.limit_idx_y);
		*/

		// client_rect 와 scale, zone grid size 를 비교하여 몇칸의 zone 을 그릴 지 결정한다.
		const double scale_ratio = scale_pcnt_ / 100.;
		const auto cen_scr_x = center_screen_x_.load();
		const auto cen_scr_y = center_screen_y_.load();

		// field 상에서의 screen size
		const int scr_width = int(client_rect.right / scale_ratio);
		const int scr_height = int(client_rect.bottom / scale_ratio);
		const int scr_x = cen_scr_x - scr_width / 2;
		const int scr_y = cen_scr_y - scr_height / 2;
		draw_only_on_screen(
			memdc, scr_x, scr_y,
			scr_width, scr_height);

		::StretchBlt(
			wdc.Get(),
			0, 0,
			client_rect.right,
			client_rect.bottom,
			memdc.Get(),
			scr_x,
			scr_y,
			scr_width,
			scr_height,
			SRCCOPY);

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

	//FIXME: !!! player_ 의 exchange 가 atomic 해야함
	// lock 을 추가하던지 뭔가 코드 수정 필요.
	class ClientPlayerInfo
	{
	public:
		void set_player_handle(uintptr_t handle)
		{
			player_handle_ = handle;
		}
		void SetPlayerHandleZero()
		{
			player_handle_ = 0;
			idx_zone_player_x = -1;
			idx_zone_player_y = -1;
		}
		void SetPlayerPosition(int idx_x, int idx_y, int x, int y)
		{
			/*idx_zone_player_x = idx_x;
			idx_zone_player_y = idx_y;*/
			player_x_.store(x);
			player_y_.store(y);
		}

		Handle<Snake>::Type player_handle_{ 0 };
		std::atomic<int> player_x_{ 0 };
		std::atomic<int> player_y_{ 0 };

		int idx_zone_player_x{ -1 };
		int idx_zone_player_y{ -1 };

		// 기존의 key 와 동일하면 false, 다르면 true
		bool SetKeyDown(PlayerKey pk)
		{
			if (player_key_ == pk) return false;

			player_key_ = pk;
			return true;
		}
		bool SetKeyUp(PlayerKey pk)
		{
			if (player_key_ != pk) return false;

			player_key_ = PlayerKey::PK_NONE;
			return true;
		}
		PlayerKey player_key_{ PlayerKey::PK_NONE };
	};
	std::shared_ptr<ClientPlayerInfo> player_info_;
	//

protected:
	struct ZoneInfo;

	void initialize(int h, int w, int cnt_x, int cnt_y)
	{
		zone_info_.h = h;
		zone_info_.w = w;
		zone_info_.limit_idx_x = cnt_x;
		zone_info_.limit_idx_y = cnt_y;

		center_screen_x_.store(h / 2);
		center_screen_y_.store(w / 2);

		zone_info_.initialized.store(true);
	}

	void retrieve_changed_list(std::map<std::pair<int, int>, std::atomic<bool>>& changed)
	{
		std::lock_guard<std::mutex> lock(clone_list_mutex_);
		changed = std::move(zone_clone_list_changed_);
	}

	void retrieve_object_list(
		int idx_x, int idx_y,
		SbGeoZone::CloneZoneObjListT& snake_clone_list,
		SbGeoZone::CloneZoneObjListT& apple_clone_list)
	{
		std::lock_guard<std::mutex> lock(clone_list_mutex_);

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

	void draw_only_on_screen(Win::WDC& memdc, int scr_x, int scr_y, int scr_width, int scr_height)
	{
		auto draw_zone_min_x = int((scr_x) / ((double)zone_info_.w));
		auto draw_zone_max_x = int((scr_x + scr_width) / ((double)zone_info_.w));
		auto draw_zone_min_y = int((scr_y) / ((double)zone_info_.h));
		auto draw_zone_max_y = int((scr_y + scr_height) / ((double)zone_info_.h));

		//
		cached_draw_zone_min_x_.store(draw_zone_min_x);
		cached_draw_zone_max_x_.store(draw_zone_max_x);
		cached_draw_zone_min_y_.store(draw_zone_min_y);
		cached_draw_zone_max_y_.store(draw_zone_max_y);
		//

		// left, top, right, bottom
		RECT draw_zone_idx{
			std::max(draw_zone_min_x, 0),
			std::max(draw_zone_min_y, 0),
			std::min(draw_zone_max_x, zone_info_.limit_idx_x - 1),
			std::min(draw_zone_max_y, zone_info_.limit_idx_y - 1) };

		std::map<std::pair<int, int>, std::atomic<bool>> changed;
		this->retrieve_changed_list(changed);

		for (int idx_x = draw_zone_idx.left; idx_x <= draw_zone_idx.right; ++idx_x)
		{
			for (int idx_y = draw_zone_idx.top; idx_y <= draw_zone_idx.bottom; ++idx_y)
			{
				auto it = changed.find(std::make_pair(idx_x, idx_y));
				if (it != changed.end() && it->second.load())
				{
					draw_grid(memdc, idx_x, idx_y);
				}
			}
		}

		for (int idx_x = draw_zone_idx.left; idx_x <= draw_zone_idx.right; ++idx_x)
		{
			for (int idx_y = draw_zone_idx.top; idx_y <= draw_zone_idx.bottom; ++idx_y)
			{
				auto it = changed.find(std::make_pair(idx_x, idx_y));
				if (it != changed.end() && it->second.load())
				{
					draw_zone(memdc, idx_x, idx_y);
				}
			}
		}
	}

	void draw_grid(Win::WDC& memdc, int idx_x, int idx_y)
	{
		if (zone_info_.initialized.load() == false)
		{
			return;
		}

		RECT zone_rect = { zone_info_.w * idx_x, zone_info_.h * idx_y,
			zone_info_.w * (idx_x + 1), zone_info_.h * (idx_y + 1) };

		// 테두리 그리기
		/*if (bGridOn_ == false)
		{
			return;
		}*/

		auto player_info = std::atomic_load(&player_info_);

		if (player_info &&
			player_info->idx_zone_player_x == idx_x &&
			player_info->idx_zone_player_y == idx_y)
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

	void draw_zone(Win::WDC& memdc, int idx_x, int idx_y)
	{
		if (zone_info_.initialized.load() == false)
		{
			return;
		}

		// snake 와 apple 의 복제본 리스트를 받아온 뒤 화면에 그린다.
		// 락을 짧은 순간만 걸기 때문에 효과적이라고 생각한다.
		SbGeoZone::CloneZoneObjListT snake_pairs;
		SbGeoZone::CloneZoneObjListT apples;
		retrieve_object_list(idx_x, idx_y, snake_pairs, apples);
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
				draw_moving_object(memdc, pair.second.head_);
				draw_object_zone_idx(memdc, pair.second, 7);
			}
			::SetTextColor(memdc.Get(), oldtextcol);
			::SelectObject(memdc.Get(), oldbrush);

			auto player_info = std::atomic_load(&player_info_);

			for (auto& snake_pair : snake_pairs)
			{
				if (player_info && snake_pair.first == player_info->player_handle_)
				{
					player_info->idx_zone_player_x = snake_pair.second.zone_idx_x_;
					player_info->idx_zone_player_y = snake_pair.second.zone_idx_y_;

					/*auto& player_pos = snake_pair.second.head_.GetPosition();
					player_x_.store(player_pos.x);
					player_y_.store(player_pos.y);*/

					HBRUSH oldbrush = (HBRUSH)::SelectObject(memdc.Get(), ::GetStockObject(BLACK_BRUSH));
					draw_snake(memdc, snake_pair.second);
					(HBRUSH)::SelectObject(memdc.Get(), oldbrush);
				}
				else
				{
					draw_snake(memdc, snake_pair.second);
				}
			}
		}
	}

	//@need GameSession::snakes_mutex_ locked
	static void draw_snake(Win::WDC& wdc, ZoneObjectClone& snake)
	{
		draw_snake_name(wdc, snake);
		draw_object_zone_idx(wdc, snake, 35);

		draw_moving_object(wdc, snake.head_);
		for (auto& body : snake.body_list_)
		{
			draw_moving_object(wdc, body);
		}
	}
	static void draw_moving_object(Win::WDC& wdc, MovingObject& mo)
	{
		const int radius = (int)mo.GetRadius();
		const auto pos = mo.GetPosition();
		const int center_x = (int)pos.x;
		const int center_y = (int)pos.y;

		wdc.Ellipse(center_x - radius, center_y - radius,
			center_x + radius, center_y + radius);
	}
	static void draw_snake_name(Win::WDC& wdc, ZoneObjectClone& snake)
	{
		auto pos = snake.head_.GetPosition();
		RECT rect{
			(long)pos.x - 50, (long)pos.y - 20,
			(long)pos.x + 50, (long)pos.y - 5
		};
		wdc.DrawTextA(snake.Name, rect, DT_CENTER);
	}
	static void draw_object_zone_idx(Win::WDC& wdc, ZoneObjectClone& obj, int top)
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

	// game field 를 화면에 몇배로 확대해서 그릴 지. 단위=퍼센트
	std::atomic<int> scale_pcnt_{ 100 };
	// game field 상에 화면의 left, top 위치
	std::atomic<int> center_screen_x_{ 0 };
	std::atomic<int> center_screen_y_{ 0 };

	std::atomic<int> cached_draw_zone_min_x_{ 0 };
	std::atomic<int> cached_draw_zone_max_x_{ 0 };
	std::atomic<int> cached_draw_zone_min_y_{ 0 };
	std::atomic<int> cached_draw_zone_max_y_{ 0 };

	std::atomic<bool> bGridOn_{ true };
	std::atomic<bool> follow_player_{ true };
	std::atomic<bool> screen_on_{ true };
};

}
}
#endif //SNAKEBITE_GAME_CLIENT_BASE_H_