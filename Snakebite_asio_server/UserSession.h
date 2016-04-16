#ifndef SNAKEBITE_USER_SESSION_H_
#define SNAKEBITE_USER_SESSION_H_

#include <memory>

#include <acoross/snakebite/win/targetver.h>
#include <boost/asio.hpp>
#include <queue>

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
		std::shared_ptr<GameSession> game_session, std::shared_ptr<GameServer> server)
		: game_session_(game_session)
		, Service(io_service, std::move(socket))
	{}

	~UserSession()
	{
		end();
	}

	void start();
	void end();

private:
	void send_update_game_object(
		const std::list<std::pair<Handle<Snake>::Type, GameObjectClone>>& snake_clone_list, 
		const std::list<GameObjectClone>& apple_clone_list);
	
	Handle<Snake>::Type user_snake_handle_;
	std::shared_ptr<GameSession> game_session_;
	
	// Service을(를) 통해 상속됨
	virtual acoross::rpc::ErrCode InitPlayer(const acoross::snakebite::messages::InitPlayerSnakeRequest &rq, acoross::snakebite::messages::InitPlayerSnakeReply *rp) override;
	virtual acoross::rpc::ErrCode SetKeyDown(const acoross::snakebite::messages::TurnKeyDownRequest &rq, acoross::snakebite::messages::VoidReply *rp) override;
	virtual acoross::rpc::ErrCode SetKeyUp(const acoross::snakebite::messages::TurnKeyUpRequest &rq, acoross::snakebite::messages::VoidReply *rp) override;
};

}
}
#endif //SNAKEBITE_USER_SESSION_H_