#include <string>

#include "UserSession.h"
#include "game_server.h"
#include <acoross/snakebite/protos/sc_snakebite_message.pb.h>
#include <acoross/snakebite/protos/snakebite_message_type.h>

namespace acoross {
namespace snakebite {

void UserSession::start()
{
	std::string myid = std::to_string((uintptr_t)this);

	game_server_.AddUpdateEventListner(
		myid, 
		[us_wp = std::weak_ptr<UserSession>(shared_from_this())](std::shared_ptr<SnakebiteMessage>& msg)
	{
		if (auto us = us_wp.lock())
		{
			us->send(msg);
		}
	});

	do_read_header();
}

void UserSession::end()
{
	if (auto my_snake = user_snake_.lock())
	{
		game_session_->RemoveSnake(Handle<Snake>(my_snake.get()).handle);
	}
	
	std::string myid = std::to_string((uintptr_t)this);
	game_server_.UnregisterEventListner(myid);
}

void UserSession::RequestInitPlayer(std::string name)
{
	if (auto player = user_snake_.lock())
	{
		game_session_->RemoveSnake(Handle<Snake>(player.get()).handle);
	}
	user_snake_ = game_session_->AddSnake(Snake::EventHandler(), name);
}

}
}
