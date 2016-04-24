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
	using LocalUpdateListner = 
		std::function<void(
			int idx_x, int idx_y, 
			SbGeoZone::CloneZoneObjListT&,
			SbGeoZone::CloneZoneObjListT&
		)>;
	
public:
	GameServer(boost::asio::io_service& io_service
		, short port
		, short push_port
	);
	~GameServer() {}

	void SetLocalUpdateListner(LocalUpdateListner local_listner)
	{
		game_session_->AddUpdateEventListner("local listner",
			[local_listner]	(
				int idx_x, int idx_y, 
				SbGeoZone::CloneZoneObjListT& snake_clone_list, 
				SbGeoZone::CloneZoneObjListT& apple_clone_list)
			{
				local_listner(idx_x, idx_y, 
					snake_clone_list, apple_clone_list);
			});
	}

	void RequestToSession(std::function<void(GameSession&)> request)
	{
		request(*game_session_);
	}

	void RequestToSessionNpcController(std::function<void(SnakeNpcControlManager&)> request)
	{
		request(*npc_controll_manager_);
	}

	void UnregisterUserSession(std::string addr)
	{
		std::lock_guard<std::recursive_mutex> lock(user_session_mutex_);
		user_session_map_.erase(addr);
	}

public:
	const int FRAME_TICK{ 100 };
#ifdef _DEBUG
	const int COUNT_ZONE_X = 20;
	const int COUNT_ZONE_Y = 20;
	const int ZoneWidth{ 100 };
	const int ZoneHeight{ 100 };
#else
	const int COUNT_ZONE_X = 40;
	const int COUNT_ZONE_Y = 40;
	const int ZoneWidth{ 100 };
	const int ZoneHeight{ 100 };
#endif
	const int ZoneGridWidth{ ZoneWidth * COUNT_ZONE_X };
	const int ZoneGridHeight{ ZoneHeight * COUNT_ZONE_X };
	
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