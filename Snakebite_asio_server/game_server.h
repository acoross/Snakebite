#ifndef SNAKEBITE_GAME_SERVER_H_
#define SNAKEBITE_GAME_SERVER_H_

#include <acoross/snakebite/win/WinWrapper.h>
#include <iostream>
#include <exception>
#include <memory>
#include <atomic>
#include <deque>
#include <map>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <acoross/snakebite/util.h>
#include <acoross/snakebite/game_session/game_session_system.h>
#include <acoross/snakebite/snake_npc_control_manager.h>
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
			const std::list<std::pair<Handle<Snake>::Type, GameObjectClone>>&, 
			const std::list<GameObjectClone>&
		)>;
	
	const int FRAME_TICK{ 100 };

public:
	GameServer(boost::asio::io_service& io_service
		, short port
		, short push_port
		)
		: io_service_(io_service)
		, rpc_server_(
			io_service, 
			port, 
			[this](boost::asio::io_service& ios, tcp::socket&& socket) { 
				on_accept(ios, std::move(socket)); 
			}
		)
		, push_server_(
			io_service, 
			push_port, 
			[this](boost::asio::io_service& ios, tcp::socket&& socket) { 
				on_accept_push_socket(ios, std::move(socket)); 
			}
		)
		, game_update_timer_(io_service, boost::posix_time::milliseconds(FRAME_TICK))
		, game_session_(std::make_unique<GameSession>(io_service, ZoneWidth, ZoneHeight, COUNT_ZONE_X, COUNT_ZONE_Y))
		, npc_controll_manager_(std::make_unique<SnakeNpcControlManager>(game_session_))
	{
		do_update_game_session();
	}
	
	void SetLocalUpdateListner(LocalUpdateListner local_listner)
	{
		game_session_->AddUpdateEventListner("local listner",
			[local_listner]	(
				const std::list<std::pair<Handle<Snake>::Type, GameObjectClone>>& snake_clone_list, 
				const std::list<GameObjectClone>& apple_clone_list)
			{
				local_listner(snake_clone_list, apple_clone_list);
			});
	}

	void RequestToSession(std::function<void(GameSession&)> request)
	{
		io_service_.post(
			[game_session = game_session_, request]()
		{
			request(*game_session);
		});
	}

	void RequestToSessionNpcController(std::function<void(SnakeNpcControlManager&)> request)
	{
		io_service_.post(
			[npc_controll_manager = npc_controll_manager_, request]()
		{
			request(*npc_controll_manager);
		});
	}

	void UnregisterUserSession(std::string addr)
	{
		std::lock_guard<std::recursive_mutex> lock(user_session_mutex_);
		user_session_map_.erase(addr);
	}

public:
	const int COUNT_ZONE_X = 1;
	const int COUNT_ZONE_Y = 1;
	const int ZoneWidth{ 600 };
	const int ZoneHeight{ 600 };
	const int ZoneGridWidth{ ZoneWidth * COUNT_ZONE_X };
	const int ZoneGridHeight{ ZoneHeight * COUNT_ZONE_X };
	
	std::atomic<double> mean_move_time_ms_{ 0 };
	std::atomic<double> mean_collision_time_ms_{ 0 };
	std::atomic<double> mean_clone_object_time_ms_{ 0 };
	std::atomic<double> mean_tick_time_ms_{ 0 };
	std::atomic<double> mean_frame_tick_{ 0 };

private:
	void do_update_game_session();

	void on_accept(
		boost::asio::io_service& io_service, 
		tcp::socket&& socket)
	{
		auto addr = socket.remote_endpoint().address().to_string();
		auto us = std::make_shared<UserSession>(
			io_service,
			std::move(socket),
			game_session_,
			shared_from_this(),
			[gs_wp = std::weak_ptr<GameServer>(shared_from_this()), addr]
			{
				if (auto gs = gs_wp.lock())
				{
					gs->UnregisterUserSession(addr);
				}
			});
		user_session_map_[addr] = us;
		us->start();
	}

	void on_accept_push_socket(
		boost::asio::io_service& io_service, 
		tcp::socket&& push_socket)
	{
		auto addr = push_socket.remote_endpoint().address().to_string();
		std::lock_guard<std::recursive_mutex> lock(user_session_mutex_);
		auto it = user_session_map_.find(addr);
		if (it != user_session_map_.end())
		{
			if (auto us = it->second.lock())
			{
				us->init_push_stub_socket(io_service, std::move(push_socket));
			}
		}
	}

private:
	boost::asio::io_service& io_service_;
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