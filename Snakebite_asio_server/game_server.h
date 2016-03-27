#ifndef SNAKEBITE_GAME_SERVER_H_
#define SNAKEBITE_GAME_SERVER_H_

#include <acoross/snakebite/win/WinWrapper.h>
#include <iostream>
#include <exception>
#include <memory>
#include <atomic>
#include <deque>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <acoross/snakebite/util.h>
#include <acoross/snakebite/game_session.h>
#include <acoross/snakebite/snake.h>
#include <acoross/snakebite/protos/snakebite_message.h>
#include "UserSession.h"

using boost::asio::ip::tcp;

namespace acoross {
namespace snakebite {

//----------------------------------------------------------------------

class GameServer final
	: public std::enable_shared_from_this<GameServer>
{
public:
	using EventHandler = std::function<void(std::shared_ptr<SnakebiteMessage>&)>;
	using LocalUpdateListner = 
		std::function
		<void(std::list<std::pair<Handle<Snake>::Type, GameObjectClone>>, std::list<GameObjectClone>)>;
	
	const int FRAME_TICK{ 33 };

public:
	GameServer(boost::asio::io_service& io_service
		, short port
		)
		: io_service_(io_service)
		, acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
		, socket_(io_service)
		, game_update_timer_(io_service, boost::posix_time::milliseconds(FRAME_TICK))
		, game_session_(std::make_unique<GameSession>(20, Width, Height))
	{
		do_update_game_session();
		do_accept();
	}
	
	void SetLocalUpdateListner(LocalUpdateListner local_listner)
	{
		on_update_local_listner_ = local_listner;
	}

	void AddUpdateEventListner(std::string name, EventHandler on_update)
	{
		std::lock_guard<std::mutex> lock(update_handler_mutex_);
		on_update_event_listeners_[name] = on_update;
	}

	void UnregisterEventListner(std::string name)
	{
		std::lock_guard<std::mutex> lock(update_handler_mutex_);
		on_update_event_listeners_.erase(name);
	}

	void RequestToSession(std::function<void(GameSession&)> request)
	{
		io_service_.post(
			[game_session = game_session_, request]()
		{
			request(*game_session);
		});
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
				std::make_shared<UserSession>(std::move(socket_), game_session_, shared_from_this())->start();
			}

			do_accept();
		});
	}

private:
	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
	tcp::socket socket_;
	
	boost::asio::deadline_timer game_update_timer_;
	std::shared_ptr<GameSession> game_session_;

	std::mutex update_handler_mutex_;
	std::map<std::string, EventHandler> on_update_event_listeners_;
	LocalUpdateListner	on_update_local_listner_;
};

}
}
#endif //SNAKEBITE_GAME_SERVER_H_