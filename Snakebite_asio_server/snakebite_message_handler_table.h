#ifndef SNAKEBITE_SNAKEBITE_MESSAGE_HANDLER_TABLE_H_
#define SNAKEBITE_SNAKEBITE_MESSAGE_HANDLER_TABLE_H_

#include <acoross/snakebite/snake.h>
#include "snakebite_message.h"

#include <acoross/snakebite/protos/snakebite_message.pb.h>

namespace acoross {
namespace snakebite {

class UserSession;

class SnakebiteMessageHandlerTable
{
public:
	bool ProcessMessage(UserSession& session, const SnakebiteMessage& request, SnakebiteMessage* reply);

	bool TurnKeyDownMessage(UserSession& session, TurnKeyDownRequest& rq);

	bool TurnKeyUpMessage(UserSession& session, TurnKeyUpRequest& rq);

	bool InitPlayerSnakeMessage(UserSession& session, InitPlayerSnakeRequest& rq);
};

}
}
#endif //SNAKEBITE_SNAKEBITE_MESSAGE_HANDLER_TABLE_H_