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
		std::function<void(void)> on_destroy)
		: game_session_(game_session)
		, Service(io_service, std::move(socket))
		, on_destroy_(on_destroy)
	{}

	~UserSession()
	{
		on_destroy_();
		end();
	}

	void start();
	void end();

	void init_push_stub_socket(::boost::asio::io_service& io_service,
		::boost::asio::ip::tcp::socket&& socket)
	{
		push_stub_.reset(new messages::SC_PushService::Stub(io_service, std::move(socket)));
	}

private:
	void send_update_game_object(
		const std::list<std::pair<Handle<Snake>::Type, GameObjectClone>>& snake_clone_list, 
		const std::list<GameObjectClone>& apple_clone_list);
	
	Handle<Snake>::Type user_snake_handle_;
	std::shared_ptr<GameSession> game_session_;
	
	std::function<void(void)> on_destroy_;

	std::shared_ptr<messages::SC_PushService::Stub> push_stub_;

	// Service을(를) 통해 상속됨
	virtual acoross::rpc::ErrCode InitPlayer(const acoross::snakebite::messages::InitPlayerSnakeRequest &rq, acoross::snakebite::messages::InitPlayerSnakeReply *rp) override;
	virtual acoross::rpc::ErrCode SetKeyDown(const acoross::snakebite::messages::TurnKeyDownRequest &rq, acoross::snakebite::messages::VoidReply *rp) override;
	virtual acoross::rpc::ErrCode SetKeyUp(const acoross::snakebite::messages::TurnKeyUpRequest &rq, acoross::snakebite::messages::VoidReply *rp) override;
};

}
}
#endif //SNAKEBITE_USER_SESSION_H_