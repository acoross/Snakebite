#ifndef SNAKEBITE_USER_SESSION_H_
#define SNAKEBITE_USER_SESSION_H_

#include <memory>
#include <boost/asio.hpp>

#include "snakebite_message.h"

#include <acoross/snakebite/protos/snakebite_protocol.pb.h>

using boost::asio::ip::tcp;

namespace acoross {
namespace snakebite {

class UserSession;

class MessageHandlerTable
{
public:
	bool ProcessMessage(
		UserSession& session, unsigned short message_type, char* data_body, size_t body_length, 
		char* reply_buffer, size_t reply_length, size_t* reply_body_length)
	{
		if (reply_body_length == nullptr)
		{
			return false;
		}

		enum class SnakebiteMessageType : unsigned short
		{
			None = 0,
			Turn,
			Max
		};

		if (message_type >= static_cast<unsigned short>(SnakebiteMessageType::Max))
		{
			return false;
		}

		auto message_type_typed = static_cast<SnakebiteMessageType>(message_type);
		switch (message_type_typed)
		{
		case SnakebiteMessageType::Turn:
			{
				TurnKeyDownRequest rq;
				rq.ParseFromArray(data_body, body_length);

				TurnKeyDownReply rp;
				
				bool ret = TurnMessage(session, rq, &rp);
				rp.SerializeToArray(reply_buffer, reply_length);
				*reply_body_length = rp.ByteSize();

				return ret;
			}
			break;
		default:
			break;
		}

		return false;
	}

	bool TurnMessage(UserSession& session, TurnKeyDownRequest rq, TurnKeyDownReply* rp)
	{
		

		return true;
	}
};

//----------------------------------------------------------------------

using SnakebiteMessageQueue = std::deque<SnakebiteMessage>;

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
		end();
	}

	void start()
	{
		user_snake_ = game_session_->AddSnake();
		do_read_header();
	}

	void end()
	{
		if (auto my_snake = user_snake_.lock())
		{
			game_session_->RemoveSnake(Handle<Snake>(my_snake.get()).handle);
		}
	}

	void send(const SnakebiteMessage& msg)
	{
		bool write_in_progress = !write_msgs_.empty();
		write_msgs_.push_back(msg);
		if (!write_in_progress)
		{
			do_write();
		}
	}

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
			boost::asio::buffer(write_msgs_.front().data(),
				write_msgs_.front().length()),
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

	bool process_message(SnakebiteMessage msg)
	{
		// 3~4 byte ดย packet number;
		SnakebiteMessage reply;
		size_t reply_body_length = 0;

		return 
			message_handler_.ProcessMessage(
			*this, 
			msg.message_type(), msg.body(), msg.body_length(), 
			reply.body(), reply.max_body_length, &reply_body_length);
	}

	tcp::socket socket_;
	/*enum { max_length = 1024 };
	char data_[max_length];*/
	std::weak_ptr<Snake> user_snake_;
	std::shared_ptr<GameSession> game_session_;

	SnakebiteMessage read_msg_;
	SnakebiteMessageQueue write_msgs_;

	MessageHandlerTable message_handler_;
};

}
}
#endif //SNAKEBITE_USER_SESSION_H_