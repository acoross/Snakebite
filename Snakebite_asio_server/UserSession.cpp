#include "UserSession.h"
#include "game_server.h"

void acoross::snakebite::UserSession::start()
{
	do_read_header();
}

inline void acoross::snakebite::UserSession::end()
{
	if (auto my_snake = user_snake_.lock())
	{
		game_session_->RemoveSnake(Handle<Snake>(my_snake.get()).handle);
	}
}

void acoross::snakebite::UserSession::RequestInitPlayer(std::string name)
{
	user_snake_ = game_session_->AddSnake(Snake::EventHandler(), name);
}
