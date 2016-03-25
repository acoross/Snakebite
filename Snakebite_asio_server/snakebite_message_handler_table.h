#ifndef SNAKEBITE_SNAKEBITE_MESSAGE_HANDLER_TABLE_H_
#define SNAKEBITE_SNAKEBITE_MESSAGE_HANDLER_TABLE_H_

#include <acoross/snakebite/snake.h>
#include <acoross/snakebite/protos/snakebite_message.h>
#include <acoross/snakebite/protos/snakebite_message.pb.h>

namespace acoross {
namespace snakebite {

class UserSession;

class SnakebiteMessageHandlerTable
{
public:
	bool ProcessMessage(UserSession& session, const SnakebiteMessage& request, SnakebiteMessage* reply);

	bool TurnKeyDownMessage(UserSession& session, messages::TurnKeyDownRequest& rq);

	bool TurnKeyUpMessage(UserSession& session, messages::TurnKeyUpRequest& rq);

	bool InitPlayerSnakeMessage(UserSession& session, messages::InitPlayerSnakeRequest& rq);
};

}
}
#endif //SNAKEBITE_SNAKEBITE_MESSAGE_HANDLER_TABLE_H_