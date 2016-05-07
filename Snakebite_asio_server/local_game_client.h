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
	virtual ~LocalGameClient() {}

	virtual void RequestZoneInfo() override
	{
		initialize(
			game_server_.ZoneWidth,
			game_server_.ZoneHeight,
			game_server_.COUNT_ZONE_X,
			game_server_.COUNT_ZONE_Y);
	}

	void SetObjectList_FilteredByCurrentObservingZoneOnly(
		int idx_x, int idx_y,
		SbGeoZone::SharedCloneZoneObjlistT snake_list,
		SbGeoZone::SharedCloneZoneObjlistT apple_list)
	{
		auto min_x = cached_draw_zone_min_x_.load();
		auto max_x = cached_draw_zone_max_x_.load();
		auto min_y = cached_draw_zone_min_y_.load();
		auto max_y = cached_draw_zone_max_y_.load();

		if (min_x > idx_x || max_x < idx_x
			|| min_y > idx_y || max_y < idx_y)
		{
			return;
		}

		auto snakes = *snake_list;
		auto apples = *apple_list;
		this->SetObjectList(idx_x, idx_y, std::move(snakes), std::move(apples));
	}

	//@lock
	virtual void InitPlayer() override
	{
		game_server_.RequestToSession(
			[this](GameSession& session)
		{
			if (auto player_info = std::atomic_load(&player_info_))
			{
				uintptr_t handle = player_info->player_handle_;
				session.AsyncRemoveSnake(handle);
			}

			auto new_player_info = std::make_shared<LocalClientPlayerInfo>();
			new_player_info->player_handle_ = session.AsyncMakeNewSnake("local player",
				[this, new_player_info](Snake& snake)
			{
				new_player_info->SetPlayerHandleZero();
			});

			session.RequestToSnake(
				new_player_info->player_handle_,
				[this, new_player_info](Snake& snake)
			{
				new_player_info->auto_conn_2_mov_event =
					snake.ConnectToPositionUpdateEvent(
						[this, new_player_info](int idx_x, int idx_y, double x, double y)
				{
					SetPlayerPosition(idx_x, idx_y, (int)x, (int)y);
				});
			});

			std::atomic_exchange(&player_info_, std::shared_ptr<ClientPlayerInfo>(new_player_info));
		});
	}
	//

	//@atomic for Snake
	virtual void SetKeyDown(PlayerKey player_key) override
	{
		if (auto player_info = std::atomic_load(&player_info_))
		{
			if (!player_info->SetKeyDown(player_key))
			{
				return;
			}

			game_server_.RequestToSession(
				[player_key, handle = player_info->player_handle_](GameSession& session)
			{
				session.RequestToSnake(handle,
					[player_key](Snake& snake)
				{
					snake.SetKeyDown(player_key);
				});
			});
		}
	}

	virtual void SetKeyUp(PlayerKey player_key) override
	{
		if (auto player_info = std::atomic_load(&player_info_))
		{
			if (!player_info->SetKeyUp(player_key))
			{
				return;
			}

			game_server_.RequestToSession(
				[player_key, handle = player_info->player_handle_](GameSession& session)
			{
				session.RequestToSnake(handle,
					[player_key](Snake& snake)
				{
					snake.SetKeyUp(player_key);
				});
			});
		}
	}
	//
private:
	GameServer& game_server_;

	class LocalClientPlayerInfo : public ClientPlayerInfo
	{
	public:
		acoross::auto_connection auto_conn_2_mov_event;
	};
};

}
}
#endif //SNAKEBITE_LOCAL_GAME_CLIENT_H_