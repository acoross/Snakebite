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
	//using EventHandler = std::function<void(std::shared_ptr<SnakebiteMessage>&)>;
	using LocalUpdateListner = 
		std::function<void(
			const std::list<std::pair<Handle<Snake>::Type, GameObjectClone>>&, 
			const std::list<GameObjectClone>&)>;
	
	const int FRAME_TICK{ 100 };

public:
	GameServer(boost::asio::io_service& io_service
		, short port
		, short push_port
		)
		: io_service_(io_service)
		, acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
		, socket_(io_service)
		, push_acceptor_(io_service, tcp::endpoint(tcp::v4(), push_port))
		, push_socket_(io_service)
		, game_update_timer_(io_service, boost::posix_time::milliseconds(FRAME_TICK))
		, game_session_(std::make_unique<GameSession>(20, Width, Height))
		, npc_controll_manager_(std::make_unique<SnakeNpcControlManager>(game_session_))
	{
		do_update_game_session();
		do_accept();
		do_accept_push_service();
	}
	
	void SetLocalUpdateListner(LocalUpdateListner local_listner)
	{
		game_session_->AddUpdateEventListner("local listner",
			[local_listner]
		(const std::list<std::pair<Handle<Snake>::Type, GameObjectClone>>& snake_clone_list, 
			const std::list<GameObjectClone>& apple_clone_list)
		{
			local_listner(snake_clone_list, apple_clone_list);
		});
	}

	/*void AddUpdateEventListner(std::string name, EventHandler on_update)
	{
		std::lock_guard<std::mutex> lock(update_handler_mutex_);
		on_update_event_listeners_[name] = on_update;
	}

	void UnregisterEventListner(std::string name)
	{
		std::lock_guard<std::mutex> lock(update_handler_mutex_);
		on_update_event_listeners_.erase(name);
	}*/

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
	const int Width{ 500 };
	const int Height{ 500 };

	std::atomic<double> mean_move_time_ms_{ 0 };
	std::atomic<double> mean_collision_time_ms_{ 0 };
	std::atomic<double> mean_clone_object_time_ms_{ 0 };
	std::atomic<double> mean_tick_time_ms_{ 0 };
	std::atomic<double> mean_frame_tick_{ 0 };

private:
	void do_update_game_session();

	void do_accept()
	{
		acceptor_.async_accept(socket_,
			[this](boost::system::error_code ec)
		{
			if (!ec)
			{
				auto addr = socket_.remote_endpoint().address().to_string();
				auto us = std::make_shared<UserSession>(
					io_service_,
					std::move(socket_),
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

			do_accept();
		});
	}

	void do_accept_push_service()
	{
		push_acceptor_.async_accept(push_socket_,
			[this](boost::system::error_code ec)
		{
			auto addr = push_socket_.remote_endpoint().address().to_string();
			std::lock_guard<std::recursive_mutex> lock(user_session_mutex_);
			auto it = user_session_map_.find(addr);
			if (it != user_session_map_.end())
			{
				if (auto us = it->second.lock())
				{
					us->init_push_stub_socket(io_service_, std::move(push_socket_));
				}
			}

			do_accept_push_service();
		});
	}

private:
	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
	tcp::socket socket_;
	
	tcp::acceptor push_acceptor_;
	tcp::socket push_socket_;

	std::recursive_mutex user_session_mutex_;
	std::map<std::string, std::weak_ptr<UserSession>> user_session_map_;

	boost::asio::deadline_timer game_update_timer_;
	std::shared_ptr<GameSession> game_session_;
	std::shared_ptr<SnakeNpcControlManager> npc_controll_manager_;

	/*std::mutex update_handler_mutex_;
	std::map<std::string, EventHandler> on_update_event_listeners_;
	LocalUpdateListner	on_update_local_listner_;*/
};

}
}
#endif //SNAKEBITE_GAME_SERVER_H_