#ifndef SNAKEBITE_GAME_SERVER_H_
#define SNAKEBITE_GAME_SERVER_H_

#include <acoross/snakebite/win/WinWrapper.h>
#include <iostream>
#include <exception>
#include <memory>
#include <atomic>
#include <deque>
#include <map>

#include <SDKDDKVer.h>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <acoross/snakebite/util.h>
#include <acoross/snakebite/game_session/game_session_system.h>
#include <acoross/snakebite/snake_npc_control_manager.h>
#include <acoross/snakebite/zone_system/zone_system.h>
#include "UserSession.h"

using boost::asio::ip::tcp;

namespace acoross {
namespace snakebite {
//----------------------------------------------------------------------

class GameServer final
	: public std::enable_shared_from_this<GameServer>
{
public:
	GameServer(boost::asio::io_service& io_service,
		short port,
		short push_port);
	~GameServer() {}

	auto MakeConnectionToGlobalUpdateEvent(SbGeoZone::ObserverT local_listner)
	{
		return game_session_->MakeConnectionToUpdateEvent(local_listner);
	}

	void RequestToSession(std::function<void(GameSession&)> request)
	{
		request(*game_session_);
	}

	void RequestToSessionNpcController(
		std::function<void(SnakeNpcControlManager&)> request)
	{
		request(*npc_controll_manager_);
	}

	void RegisterUserSession(std::string addr, std::shared_ptr<UserSession> us)
	{
		std::lock_guard<std::recursive_mutex> lock(user_session_mutex_);
		user_session_map_[addr] = us;
	}
	void UnregisterUserSession(std::string addr)
	{
		std::lock_guard<std::recursive_mutex> lock(user_session_mutex_);
		user_session_map_.erase(addr);
	}
	std::shared_ptr<UserSession> FindUserSession(std::string addr)
	{
		std::lock_guard<std::recursive_mutex> lock(user_session_mutex_);
		auto it = user_session_map_.find(addr);
		if (it != user_session_map_.end())
		{
			if (auto us = it->second.lock())
			{
				return us;
			}
		}
		return std::shared_ptr<UserSession>();
	}

public:
	const int FRAME_TICK{ 100 };
#ifdef _DEBUG
	const int COUNT_ZONE_X = 10;
	const int COUNT_ZONE_Y = 10;
	const int ZoneWidth{ 500 };
	const int ZoneHeight{ 500 };
#else
	const int COUNT_ZONE_X = 2;
	const int COUNT_ZONE_Y = 2;
	const int ZoneWidth{ 500 };
	const int ZoneHeight{ 500 };
#endif
	const int ZoneGridWidth{ ZoneWidth * COUNT_ZONE_X };
	const int ZoneGridHeight{ ZoneHeight * COUNT_ZONE_X };

	double GetUpdateTime()
	{
		static int64_t lasttick = ::GetTickCount();
		if (::GetTickCount() > lasttick + 200)
		{
			total_zone_update_time_ms_.store(game_session_->CalculateTotalZoneUpdateTime());
		}
		return total_zone_update_time_ms_.load();
	}

	double GetBroadcastTime()
	{
		static int64_t lasttick = ::GetTickCount();
		if (::GetTickCount() > lasttick + 200)
		{
			total_zone_broadcast_time_ms_.store(game_session_->CalculateTotalBroadcastTime());
		}
		return total_zone_broadcast_time_ms_.load();
	}


	std::atomic<double> total_zone_update_time_ms_{ 0 };
	std::atomic<double> total_zone_broadcast_time_ms_{ 0 };

	std::atomic<double> mean_move_time_ms_{ 0 };
	std::atomic<double> mean_collision_time_ms_{ 0 };
	std::atomic<double> mean_clone_object_time_ms_{ 0 };
	std::atomic<double> mean_tick_time_ms_{ 0 };
	std::atomic<double> mean_frame_tick_{ 0 };

private:
	void on_accept(
		boost::asio::io_service& io_service,
		tcp::socket&& socket);

	void on_accept_push_socket(
		boost::asio::io_service& io_service,
		tcp::socket&& push_socket);

private:
	rpc::RpcServer rpc_server_;
	rpc::RpcServer push_server_;

	std::recursive_mutex user_session_mutex_;
	std::map<std::string, std::weak_ptr<UserSession>> user_session_map_;

	boost::asio::deadline_timer game_update_timer_;
	std::shared_ptr<GameSession> game_session_;
	std::shared_ptr<SnakeNpcControlManager> npc_controll_manager_;
};
}
}
#endif //SNAKEBITE_GAME_SERVER_H_