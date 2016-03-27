#ifndef SNAKEBITE_USER_SESSION_H_
#define SNAKEBITE_USER_SESSION_H_

#include <acoross/snakebite/win/WinWrapper.h>

#include <memory>
#include <boost/asio.hpp>
#include <deque>

#include <acoross/snakebite/game_session/game_session_system.h>

#include <acoross/snakebite/protos/snakebite_message.h>
#include <acoross/snakebite/protos/snakebite_message.pb.h>
#include "snakebite_message_handler_table.h"

using boost::asio::ip::tcp;

namespace acoross {
namespace snakebite {

using SnakebiteMessageQueue = std::deque<std::shared_ptr<SnakebiteMessage>>;

class GameServer;

class UserSession
	: public std::enable_shared_from_this<UserSession>
{
public:
	UserSession(tcp::socket socket, std::shared_ptr<GameSession> game_session, std::shared_ptr<GameServer> server)
		: socket_(std::move(socket))
		, game_session_(game_session)
		, game_server_(server)
	{}

	~UserSession()
	{
		end();
	}

	void start();

	void end();

	void send(const std::shared_ptr<SnakebiteMessage>& msg)
	{
		bool write_in_progress = !write_msgs_.empty();
		write_msgs_.push_back(msg);
		if (!write_in_progress)
		{
			do_write();
		}
	}

	void TurnKeyDown(PlayerKey pk);

	void TurnKeyUp(PlayerKey pk);

	Handle<Snake>::Type RequestInitPlayer(std::string name);

private:
	void do_read_header()
	{
		auto self(shared_from_this());
		boost::asio::async_read(socket_,
			boost::asio::buffer(read_msg_.data(), SnakebiteMessage::header_length),
			[this, self](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec && read_msg_.decode_header())
			{
				do_read_body();
			}
			else
			{
				//room_.leave(shared_from_this());
				end();
			}
		});
	}

	void do_read_body()
	{
		auto self(shared_from_this());
		boost::asio::async_read(socket_,
			boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
			[this, self](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec && process_message(read_msg_))
			{
				//room_.deliver(read_msg_);
				do_read_header();
			}
			else
			{
				//room_.leave(shared_from_this());
				end();
			}
		});
	}

	void do_write()
	{
		auto self(shared_from_this());
		boost::asio::async_write(socket_,
			boost::asio::buffer(write_msgs_.front()->data(),
				write_msgs_.front()->length()),
			[this, self](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				write_msgs_.pop_front();
				if (!write_msgs_.empty())
				{
					do_write();
				}
			}
			else
			{
				//room_.leave(shared_from_this());
				end();
			}
		});
	}

	bool process_message(SnakebiteMessage& msg)
	{
		std::unique_ptr<SnakebiteMessage> reply = nullptr;
		bool ret = message_handler_.ProcessMessage(*this, msg, reply);

		if (reply)
		{
			send(std::move(reply));
		}

		return ret;
	}

	tcp::socket socket_;
	
	Handle<Snake>::Type user_snake_handle_;
	std::shared_ptr<GameSession> game_session_;
	std::shared_ptr<GameServer> game_server_;

	SnakebiteMessage read_msg_;
	SnakebiteMessageQueue write_msgs_;

	SnakebiteMessageHandlerTable message_handler_;
};

}
}
#endif //SNAKEBITE_USER_SESSION_H_