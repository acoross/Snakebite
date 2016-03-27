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

	game_server_->AddUpdateEventListner(
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
	game_session_->RemoveSnake(user_snake_handle_);

	std::string myid = std::to_string((uintptr_t)this);
	game_server_->UnregisterEventListner(myid);
}


// @atomic

void UserSession::TurnKeyDown(PlayerKey pk)
{
	game_session_->RequestToSnake(user_snake_handle_,
		[pk](Snake& snake)
	{
		snake.SetKeyDown(pk);
	});
}

void UserSession::TurnKeyUp(PlayerKey pk)
{
	game_session_->RequestToSnake(user_snake_handle_,
		[pk](Snake& snake)
	{
		snake.SetKeyUp(pk);
	});
}

Handle<Snake>::Type UserSession::RequestInitPlayer(std::string name)
{
	game_session_->RemoveSnake(user_snake_handle_);
	
	user_snake_handle_ = game_session_->AddSnake(Snake::EventHandler(), name);
	return user_snake_handle_;
}

}
}
