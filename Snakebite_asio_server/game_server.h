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
#include "snakebite_message.h"

using boost::asio::ip::tcp;

namespace acoross {
namespace snakebite {

//----------------------------------------------------------------------

using SnakebiteMessageQueue =  std::deque<SnakebiteMessage>;

//----------------------------------------------------------------------

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

		return false; // Á¾·á
	}

	tcp::socket socket_;
	enum { max_length = 1024 };
	char data_[max_length];
	std::weak_ptr<Snake> user_snake_;
	std::shared_ptr<GameSession> game_session_;

	SnakebiteMessage read_msg_;
	SnakebiteMessageQueue write_msgs_;
};

class GameServer
{
public:
	using EventHandler = std::function<void(GameSession&)>;

	GameServer(boost::asio::io_service& io_service
		, short port
		)
		: io_service_(io_service)
		, acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
		, socket_(io_service)
		, game_update_timer_(io_service, boost::posix_time::milliseconds(FRAME_TICK))
		, game_session_(std::make_unique<GameSession>(20, 500, 500))
	{
		do_update_game_session();
		do_accept();
	}
	
	void AddUpdateEventListner(std::string name, EventHandler on_update)
	{
		std::lock_guard<std::mutex> lock(update_handler_mutex_);
		on_update_event_listeners_[name] = on_update;
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
	void do_update_game_session()
	{
		MeanProcessTimeChecker mean_tick(mean_tick_time_ms_);
		game_update_timer_.expires_from_now(boost::posix_time::milliseconds(FRAME_TICK));

		static uint64_t last_tick = ::GetTickCount64();
		uint64_t current_tick = ::GetTickCount64();
		uint64_t diff = current_tick - last_tick;

		double new_mean_time = mean_frame_tick_.load() * 0.9 + diff * 0.1;
		mean_frame_tick_.store((double)new_mean_time);

		{
			MeanProcessTimeChecker mean_move(mean_move_time_ms_);
			game_session_->UpdateMove(FRAME_TICK);
		}
		
		{
			MeanProcessTimeChecker mean_collision(mean_collision_time_ms_);
			game_session_->ProcessCollisions();
		}

		{
			std::lock_guard<std::mutex> lock(update_handler_mutex_);
			MeanProcessTimeChecker mean_clone(mean_clone_object_time_ms_);
			for (auto& pair : on_update_event_listeners_)
			{
				auto& listner = pair.second;
				listner(*game_session_);
			}
		}

		last_tick = current_tick;
				
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

	const int FRAME_TICK{ 33 };

	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
	tcp::socket socket_;
	
	boost::asio::deadline_timer game_update_timer_;
	std::shared_ptr<GameSession> game_session_;

	std::mutex update_handler_mutex_;
	std::map<std::string, EventHandler> on_update_event_listeners_;
};

}
}
#endif //SNAKEBITE_GAME_SERVER_H_