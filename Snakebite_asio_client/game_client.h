#ifndef SNAKEBITE_GAME_CLIENT_H_
#define SNAKEBITE_GAME_CLIENT_H_

#include <SDKDDKVer.h>
#include <boost/asio.hpp>

#include <acoross/snakebite/protos/snakebite_message.rpc.h>

#include <acoross/snakebite/game_session/game_session_system.h>
#include <acoross/snakebite/game_client_base.h>

using boost::asio::ip::tcp;
enum { max_length = 1024 };

namespace acoross {
namespace snakebite {

class GameClient;

class SC_PushServiceImpl final
	: public messages::SC_PushService::Service
{
public:
	SC_PushServiceImpl(::boost::asio::io_service& io_service,
		::boost::asio::ip::tcp::socket&& socket,
		std::shared_ptr<GameClient> owner)
		: messages::SC_PushService::Service(io_service, std::move(socket))
		, owner_(owner)
	{}

	// Service을(를) 통해 상속됨
	virtual acoross::rpc::ErrCode QueryClientPort(
		const acoross::snakebite::messages::VoidReply &rq,
		acoross::snakebite::messages::AddressReply* rp) override;

	virtual acoross::rpc::ErrCode UpdateGameObjects(
		const acoross::snakebite::messages::UpdateGameObjectsEvent &rq,
		acoross::snakebite::messages::VoidReply *rp) override;

	virtual acoross::rpc::ErrCode ResetPlayer(
		const acoross::snakebite::messages::VoidReply &rq,
		acoross::snakebite::messages::VoidReply *rp) override;

	virtual acoross::rpc::ErrCode NotifyPlayerPosition(
		const acoross::snakebite::messages::PlayerPosition &rq,
		acoross::snakebite::messages::VoidReply *rp) override;
	//
private:
	std::shared_ptr<GameClient> owner_;
};

class GameClient final
	: public GameClientBase
	, public std::enable_shared_from_this<GameClient>
{
public:
	class AutoController
	{
	public:
		AutoController()
		{
			auto clock = std::chrono::high_resolution_clock();
			auto t = clock.now();
			random_engine_.seed((unsigned int)t.time_since_epoch().count());
		}

		void ChangeDirection(int64_t diff_in_ms, GameClient& client)
		{
			if (checkChangeDirection(diff_in_ms))
			{
				changeDirection(random_engine_, diff_in_ms, client);
			}
		}

	private:
		// 임시:
		// 랜덤하게 방향을 변경.
		// UpdatteMove 가 불린 횟수와 관계없이,
		// 시간당 방향전환 횟수가 랜덤하도록 방향을 설정.
		static bool checkChangeDirection(int64_t diff_in_ms)
		{
			static int64_t delay_sum = 0;

			delay_sum += diff_in_ms;
			if (delay_sum > 100 /*1s*/)
			{
				delay_sum = 0;
				return true;
			}

			return false;
		}

		static void changeDirection(std::default_random_engine& re, int64_t diff_in_ms, GameClient& client)
		{
			std::uniform_int_distribution<int> unin(0, 100);

			auto p = unin(re);
			if (p < 15) // 5 percent
			{
				client.SetKeyDown(PK_LEFT);
			}
			else if (p < 30) // another 5 percent
			{
				client.SetKeyDown(PK_RIGHT);
			}
			else if (p < 45)
			{
				client.SetKeyUp(PK_LEFT);
				client.SetKeyUp(PK_RIGHT);
			}
		}

		std::default_random_engine random_engine_;
	};

	GameClient(boost::asio::io_service& io_service,
		tcp::socket&& socket,
		bool auto_player = false,
		bool draw_screen = true,
		bool follow_player = true)
		: io_service_(io_service)
		, stub_(new messages::SnakebiteService::Stub(io_service, std::move(socket)))
		, timer_(io_service)
		, auto_controller_()
		, auto_player_mode_(auto_player)
	{
		screen_on_.store(draw_screen);
		follow_player_.store(follow_player);
	}

	void start()
	{
		stub_->start();
		this->RequestZoneInfo();

		if (auto_player_mode_.load())
		{
			this->InitPlayer();
		}

		on_tick(30);
	}

	void on_tick(int64_t diff_in_ms)
	{
		timer_.expires_from_now(boost::posix_time::milliseconds(diff_in_ms));

		if (zone_info_.initialized.load() && auto_player_mode_.load())
		{
			auto_controller_.ChangeDirection(diff_in_ms, *this);

			diff_in_ms = 50;
		}
		else
		{
			diff_in_ms = 100;
		}

		timer_.async_wait(
			[this, diff_in_ms](boost::system::error_code ec)
		{
			if (!ec)
			{
				on_tick(diff_in_ms);
			}
		});
	}

	void SetAutoOnOff()
	{
		auto_player_mode_.store(!auto_player_mode_.load());
	}

	std::string GetSocketAddress() const
	{
		return my_address_;
	}

	/////////////////////////////////////////////
	virtual void RequestZoneInfo() override
	{
		messages::VoidReply rq;

		stub_->RequestZoneInfo(
			rq,
			[this, client = shared_from_this()](acoross::rpc::ErrCode ec, messages::ZoneInfoReply& rp)
		{
			if (ec == acoross::rpc::ErrCode::NoError)
			{
				my_address_ = rp.my_address();
				initialize(rp.height(), rp.width(), rp.limit_idx_x(), rp.limit_idx_y());

				{
					tcp::resolver resolver(io_service_);
					boost::asio::ip::tcp::socket push_service_socket(io_service_);

					boost::asio::connect(push_service_socket, resolver.resolve({ "127.0.0.1", "22001" }));
					push_service_ = std::make_shared<SC_PushServiceImpl>(
						io_service_,
						std::move(push_service_socket),
						shared_from_this());
					push_service_->start();
				}
			}
			else
			{
				assert(false);
			}
		});
	}

	virtual void InitPlayer() override
	{
		messages::InitPlayerSnakeRequest rq;
		rq.set_name("remote player");

		stub_->InitPlayer(rq,
			[client = shared_from_this()](acoross::rpc::ErrCode ec, messages::InitPlayerSnakeReply& rp)
		{
			if (ec == acoross::rpc::ErrCode::NoError)
			{
				auto player_info = std::make_shared<ClientPlayerInfo>();
				player_info->set_player_handle(rp.handle());
				std::atomic_exchange(&client->player_info_, player_info);
			}
			else
			{
				assert(false);
			}
		});
	}

	virtual void SetKeyDown(PlayerKey pk) override
	{
		messages::TurnKeyDownRequest rq;
		rq.set_key((google::protobuf::int32)pk);

		stub_->SetKeyDown(rq,
			[client = shared_from_this()](acoross::rpc::ErrCode ec, messages::VoidReply&)
		{});
	}

	virtual void SetKeyUp(PlayerKey pk) override
	{
		messages::TurnKeyUpRequest rq;
		rq.set_key((google::protobuf::int32)pk);

		stub_->SetKeyUp(rq,
			[client = shared_from_this()](acoross::rpc::ErrCode ec, messages::VoidReply&)
		{});
	}

	bool UpdateGameObjectPositions(const messages::UpdateGameObjectsEvent& got_msg);

public:
	std::atomic<bool> auto_player_mode_{ false };

private:
	boost::asio::io_service& io_service_;
	std::shared_ptr<messages::SnakebiteService::Stub> stub_;
	std::shared_ptr<SC_PushServiceImpl> push_service_;

	::boost::asio::deadline_timer timer_;
	AutoController auto_controller_;

	std::string my_address_;
};

}
}
#endif //SNAKEBITE_GAME_CLIENT_H_