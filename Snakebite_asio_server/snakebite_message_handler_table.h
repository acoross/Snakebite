#ifndef SNAKEBITE_SNAKEBITE_MESSAGE_HANDLER_TABLE_H_
#define SNAKEBITE_SNAKEBITE_MESSAGE_HANDLER_TABLE_H_

#include <acoross/snakebite/snake.h>
#include "snakebite_message.h"

#include <acoross/snakebite/protos/snakebite_message.pb.h>

namespace acoross {
namespace snakebite {

enum class SnakebiteMessageType : unsigned short
{
	None = 0,
	TurnKeyDown,
	TurnKeyUp,
	Max
};

class UserSession;

class SnakebiteMessageHandlerTable
{
public:
	bool ProcessMessage(UserSession& session, SnakebiteMessage request, SnakebiteMessage* reply);

	bool TurnKeyDownMessage(UserSession& session, TurnKeyDownRequest rq, TurnKeyDownReply* rp);

	bool TurnKeyUpMessage(UserSession& session, TurnKeyUpRequest rq, TurnKeyUpReply* rp);
};

}
}
#endif //SNAKEBITE_SNAKEBITE_MESSAGE_HANDLER_TABLE_H_