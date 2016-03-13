#ifndef SNAKEBITE_GAME_SERVER_H_
#define SNAKEBITE_GAME_SERVER_H_

#include <acoross/snakebite/win/WinWrapper.h>
#include <iostream>
#include <exception>
#include <memory>

#include <boost/asio.hpp>

#include <acoross/snakebite/game_session.h>

using boost::asio::ip::tcp;
using acoross::snakebite::GameSession;


class UserSession
	: public std::enable_shared_from_this<UserSession>
{
public:
	UserSession(tcp::socket socket)
		: socket_(std::move(socket))
	{}

	void start()
	{
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
};

class GameServer
{
public:
	GameServer(boost::asio::io_service& io_service, short port)
		: acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
		socket_(io_service)
	{
		do_accept();
	}

private:
	void do_accept()
	{
		acceptor_.async_accept(socket_,
			[this](boost::system::error_code ec)
		{
			if (!ec)
			{
				std::make_shared<UserSession>(std::move(socket_))->start();
			}

			do_accept();
		});
	}

	tcp::acceptor acceptor_;
	tcp::socket socket_;

	GameSession game_session_;
};

#endif //SNAKEBITE_GAME_SERVER_H_