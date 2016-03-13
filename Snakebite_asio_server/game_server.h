#ifndef SNAKEBITE_GAME_SERVER_H_
#define SNAKEBITE_GAME_SERVER_H_

#include <acoross/snakebite/win/WinWrapper.h>
#include <iostream>
#include <exception>
#include <memory>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <acoross/snakebite/game_session.h>
#include <acoross/snakebite/game_session_drawer.h>
#include <acoross/snakebite/snake.h>

using boost::asio::ip::tcp;

namespace acoross {
namespace snakebite {

class UserSession
	: public std::enable_shared_from_this<UserSession>
{
public:
	UserSession(tcp::socket socket, std::shared_ptr<GameSession> game_session)
		: socket_(std::move(socket))
		, game_session_(game_session)
	{}

	~UserSession()
	{
		if (auto my_snake = user_snake_.lock())
		{
			game_session_->RemoveSnake(my_snake.get());
		}
	}

	void start()
	{
		user_snake_ = game_session_->AddSnake();
		do_read();
	}

private:
	void do_read()
	{
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(data_, max_length),
			[this, self](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				do_write(length);
			}
		});
	}

	void do_write(std::size_t length)
	{
		auto self(shared_from_this());
		boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
			[this, self](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				do_read();
			}
		});
	}

	tcp::socket socket_;
	enum { max_length = 1024 };
	char data_[max_length];
	std::weak_ptr<Snake> user_snake_;
	std::shared_ptr<GameSession> game_session_;
};

class GameServer
{
public:
	GameServer(boost::asio::io_service& io_service
		, short port
		, std::shared_ptr<GameSession> game_session
		)
		: acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
		, socket_(io_service)
		, game_update_timer_(io_service, boost::posix_time::milliseconds(FRAME_TICK))
		, game_session_(game_session)
	{
		do_update_game_session();
		do_accept();
	}

private:
	void do_update_game_session()
	{
		static uint64_t last_tick = ::GetTickCount64();
		uint64_t current_tick = ::GetTickCount64();
		uint64_t diff = current_tick - last_tick;

		//for (;diff > FRAME_TICK; diff -= FRAME_TICK)
		{
			game_session_->UpdateMove(FRAME_TICK);
			game_session_->ProcessCollisions();
		}

		last_tick = current_tick;

		game_update_timer_.expires_from_now(boost::posix_time::milliseconds(FRAME_TICK));
		game_update_timer_.async_wait(
			[this](boost::system::error_code ec)
		{
			if (!ec)
			{
				do_update_game_session();
			}
		});
	}

	void do_accept()
	{
		acceptor_.async_accept(socket_,
			[this](boost::system::error_code ec)
		{
			if (!ec)
			{
				std::make_shared<UserSession>(std::move(socket_), game_session_)->start();
			}

			do_accept();
		});
	}

	const int FRAME_TICK{ 66 };

	tcp::acceptor acceptor_;
	tcp::socket socket_;
	
	boost::asio::deadline_timer game_update_timer_;
	std::shared_ptr<GameSession> game_session_;
};

}
}
#endif //SNAKEBITE_GAME_SERVER_H_