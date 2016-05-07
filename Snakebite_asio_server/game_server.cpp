#include "game_server.h"

namespace acoross {
namespace snakebite {
GameServer::GameServer(
	boost::asio::io_service& io_service,
	short port,
	short push_port)
	: rpc_server_(io_service, port,
		[this](boost::asio::io_service& ios,
			tcp::socket&& socket)
{
	on_accept(ios, std::move(socket));
})
, push_server_(io_service, push_port,
	[this](boost::asio::io_service& ios, tcp::socket&& socket)
{
	on_accept_push_socket(ios, std::move(socket));
})
, game_update_timer_(io_service,
	boost::posix_time::milliseconds(FRAME_TICK))
	, game_session_(std::make_unique<GameSession>(
		io_service, ZoneWidth, ZoneHeight, COUNT_ZONE_X, COUNT_ZONE_Y))
	, npc_controll_manager_(
		std::make_unique<SnakeNpcControlManager>(
			io_service, game_session_))
{
	game_session_->StartZone(FRAME_TICK);
	npc_controll_manager_->Start(FRAME_TICK);
}

void GameServer::on_accept(boost::asio::io_service & io_service,
	tcp::socket && socket)
{
	auto addr = socket.remote_endpoint().address().to_string();
	addr += ":" + std::to_string(socket.remote_endpoint().port());
	auto us =
		std::make_shared<UserSession>(
			io_service,
			std::move(socket),
			game_session_,
			shared_from_this(),
			addr,
			[gs_wp = std::weak_ptr<GameServer>(shared_from_this()), addr]
	{
		if (auto gs = gs_wp.lock())
		{
			gs->UnregisterUserSession(addr);
		}
	});

	this->RegisterUserSession(addr, us);
	us->start();
}

void GameServer::on_accept_push_socket(boost::asio::io_service& io_service,
	tcp::socket&& push_socket)
{
	auto addr = push_socket.remote_endpoint().address().to_string();
	addr += ":" + std::to_string(push_socket.remote_endpoint().port());

	auto push_stub = std::make_shared<messages::SC_PushService::Stub>(io_service, std::move(push_socket));
	push_stub->start();

	messages::VoidReply rq;
	push_stub->QueryClientPort(rq,
		[&io_service, this, push_stub](rpc::ErrCode ec, messages::AddressReply& addr)
	{
		if (auto us = this->FindUserSession(addr.addr()))
		{
			us->init_push_stub(io_service, push_stub);
		}
	});
}
}
}