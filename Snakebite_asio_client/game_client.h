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
	virtual acoross::rpc::ErrCode UpdateGameObjects(
		const acoross::snakebite::messages::UpdateGameObjectsEvent &rq, 
		acoross::snakebite::messages::VoidReply *rp) override;
	
	virtual acoross::rpc::ErrCode ResetPlayer(
		const acoross::snakebite::messages::VoidReply &rq, 
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
	GameClient(boost::asio::io_service& io_service,
		tcp::socket&& socket)
		: io_service_(io_service)
		, stub_(new messages::SnakebiteService::Stub(io_service, std::move(socket)))
	{}

	void start()
	{
		stub_->start();
		this->RequestZoneInfo();
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
				zone_info_.h = rp.height();
				zone_info_.w = rp.width();
				zone_info_.limit_idx_x = rp.limit_idx_x();
				zone_info_.limit_idx_y = rp.limit_idx_y();
				zone_info_.initialized.store(true);

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
				client->set_player_handle(rp.handle());
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
	
private:
	boost::asio::io_service& io_service_;
	std::shared_ptr<messages::SnakebiteService::Stub> stub_;
	std::shared_ptr<SC_PushServiceImpl> push_service_;
};

}
}
#endif //SNAKEBITE_GAME_CLIENT_H_