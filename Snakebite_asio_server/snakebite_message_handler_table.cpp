#include "snakebite_message_handler_table.h"

#include "UserSession.h"

namespace acoross {
namespace snakebite {

bool SnakebiteMessageHandlerTable::ProcessMessage(UserSession & session, SnakebiteMessage request, SnakebiteMessage * reply)
{
	if (reply == nullptr)
	{
		return false;
	}

	if (request.message_type() >= static_cast<unsigned short>(SnakebiteMessageType::Max))
	{
		return false;
	}

	auto message_type_typed = static_cast<SnakebiteMessageType>(request.message_type());
	switch (message_type_typed)
	{
	case SnakebiteMessageType::TurnKeyDown:
	{
		TurnKeyDownRequest rq;
		rq.ParseFromArray(request.body(), request.body_length());

		TurnKeyDownReply rp;

		bool ret = TurnKeyDownMessage(session, rq, &rp);
		int reply_length = 0;
		rp.SerializeToArray(reply->body(), reply_length);
		reply->body_length((unsigned short)reply_length);

		return ret;
	}
	case SnakebiteMessageType::TurnKeyUp:
	{
		TurnKeyUpRequest rq;
		rq.ParseFromArray(request.body(), request.body_length());

		TurnKeyUpReply rp;

		bool ret = TurnKeyUpMessage(session, rq, &rp);
		int reply_length = 0;
		rp.SerializeToArray(reply->body(), reply_length);
		reply->body_length((unsigned short)reply_length);

		return ret;
	}
	default:
		break;
	}

	return false;
}

bool SnakebiteMessageHandlerTable::TurnKeyDownMessage(UserSession & session, TurnKeyDownRequest rq, TurnKeyDownReply * rp)
{
	const int key = rq.key();
	if (key < 0 || key >= PlayerKey::PK_MAX)
	{
		return false;
	}

	const PlayerKey player_key = static_cast<PlayerKey>(key);
	session.TurnKeyDown(player_key);

	return true;
}

bool SnakebiteMessageHandlerTable::TurnKeyUpMessage(UserSession & session, TurnKeyUpRequest rq, TurnKeyUpReply * rp)
{
	const int key = rq.key();
	if (key < 0 || key >= PlayerKey::PK_MAX)
	{
		return false;
	}

	const PlayerKey player_key = static_cast<PlayerKey>(key);
	session.TurnKeyUp(player_key);

	return true;
}

}
}
