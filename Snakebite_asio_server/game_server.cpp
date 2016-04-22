#include "game_server.h"

namespace acoross {
namespace snakebite {

GameServer::GameServer(
	boost::asio::io_service& io_service, 
	short port, 
	short push_port
	)
	: io_service_(io_service)
	, rpc_server_(io_service, port,
		[this](boost::asio::io_service& ios, tcp::socket&& socket) {
			on_accept(ios, std::move(socket));
		})
	, push_server_(io_service, push_port,
		[this](boost::asio::io_service& ios, tcp::socket&& socket) {
			on_accept_push_socket(ios, std::move(socket));
		})
	, game_update_timer_(io_service, boost::posix_time::milliseconds(FRAME_TICK))
	, game_session_(
		std::make_unique<GameSession>(
			io_service, ZoneWidth, ZoneHeight, COUNT_ZONE_X, COUNT_ZONE_Y))
	, npc_controll_manager_(
		std::make_unique<SnakeNpcControlManager>(
			io_service, game_session_))
	{
		game_session_->StartZone(FRAME_TICK);
		do_update_game_session();
	}

void GameServer::do_update_game_session()
{
	MeanProcessTimeChecker mean_tick(mean_tick_time_ms_);
	game_update_timer_.expires_from_now(boost::posix_time::milliseconds(FRAME_TICK));

	static uint64_t last_tick = ::GetTickCount64();
	uint64_t current_tick = ::GetTickCount64();
	uint64_t diff = current_tick - last_tick;

	double new_mean_time = mean_frame_tick_.load() * 0.9 + diff * 0.1;
	mean_frame_tick_.store((double)new_mean_time);

	{
		npc_controll_manager_->AsyncChangeNpcDirection(FRAME_TICK);
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

void GameServer::on_accept(boost::asio::io_service & io_service, tcp::socket && socket)
{
	auto addr = socket.remote_endpoint().address().to_string();
	auto us = std::make_shared<UserSession>(
		io_service,
		std::move(socket),
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

void GameServer::on_accept_push_socket(boost::asio::io_service & io_service, tcp::socket && push_socket)
{
	auto addr = push_socket.remote_endpoint().address().to_string();
	std::lock_guard<std::recursive_mutex> lock(user_session_mutex_);
	auto it = user_session_map_.find(addr);
	if (it != user_session_map_.end())
	{
		if (auto us = it->second.lock())
		{
			us->init_push_stub_socket(io_service, std::move(push_socket));
		}
	}
}

}
}
