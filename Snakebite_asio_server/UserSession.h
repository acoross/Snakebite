#ifndef SNAKEBITE_USER_SESSION_H_
#define SNAKEBITE_USER_SESSION_H_

#include <memory>

#include <acoross/snakebite/win/targetver.h>
#include <SDKDDKVer.h>
#include <boost/asio.hpp>
#include <queue>
#include <functional>

#include <acoross/snakebite/game_session/game_session_system.h>
#include <acoross/rpc/rpc_server.h>
#include <acoross/rpc/rpc_macros.h>
#include <acoross/snakebite/protos/snakebite_message.rpc.h>

namespace acoross {
namespace snakebite {
class GameServer;

class UserSession
	: public messages::SnakebiteService::Service
{
public:
	UserSession(boost::asio::io_service& io_service, ::boost::asio::ip::tcp::socket&& socket,
		std::shared_ptr<GameSession> game_session, std::shared_ptr<GameServer> server,
		std::string& my_address,
		std::function<void(void)> on_destroy)
		: game_session_(game_session)
		, Service(io_service, std::move(socket))
		, on_destroy_(on_destroy)
	{
		my_address_ = my_address;
	}

	virtual ~UserSession()
	{
		on_destroy_();
		end();
	}

	void start();
	void end();

	void init_push_stub(::boost::asio::io_service& io_service,
		std::shared_ptr<messages::SC_PushService::Stub> push_stub)
	{
		std::atomic_store(&push_stub_, push_stub);
	}

private:
	void send_update_game_object(
		int idx_x, int idx_y,
		const std::list<std::pair<Handle<Snake>::Type, ZoneObjectClone>>& snake_clone_list,
		const std::list<std::pair<Handle<Snake>::Type, ZoneObjectClone>>& apple_clone_list);
	
	Handle<Snake>::Type user_snake_handle_;
	std::atomic<int> player_idx_x_{ 0 };
	std::atomic<int> player_idx_y_{ 0 };

	std::shared_ptr<GameSession> game_session_;
	acoross::auto_connection auto_discon_observer_to_session_;
	acoross::auto_connection auto_discon_observer_to_player_;

	std::function<void(void)> on_destroy_;

	std::shared_ptr<messages::SC_PushService::Stub> push_stub_;

	std::string my_address_;

	// Service을(를) 통해 상속됨
	virtual acoross::rpc::ErrCode RequestZoneInfo(const acoross::snakebite::messages::VoidReply &rq, acoross::snakebite::messages::ZoneInfoReply *rp) override;
	virtual acoross::rpc::ErrCode InitPlayer(const acoross::snakebite::messages::InitPlayerSnakeRequest &rq, acoross::snakebite::messages::InitPlayerSnakeReply *rp) override;
	virtual acoross::rpc::ErrCode SetKeyDown(const acoross::snakebite::messages::TurnKeyDownRequest &rq, acoross::snakebite::messages::VoidReply *rp) override;
	virtual acoross::rpc::ErrCode SetKeyUp(const acoross::snakebite::messages::TurnKeyUpRequest &rq, acoross::snakebite::messages::VoidReply *rp) override;
};
}
}
#endif //SNAKEBITE_USER_SESSION_H_