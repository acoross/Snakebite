#include "snakebite_message_handler_table.h"

#include "UserSession.h"
#include <acoross/snakebite/protos/snakebite_message_type.h>
#include <acoross/snakebite/protos/snakebite_message.h>
#include <acoross/snakebite/protos/sc_snakebite_message.pb.h>

namespace acoross {
namespace snakebite {

bool SnakebiteMessageHandlerTable::ProcessMessage(UserSession& session, const SnakebiteMessage& request, std::unique_ptr<SnakebiteMessage>& /*OUT*/ reply)
{
	using namespace messages;

	/*if (reply == nullptr)
	{
		return false;
	}*/

	if (request.message_type() >= static_cast<unsigned short>(SnakebiteMessageType::Max))
	{
		return false;
	}

	bool ret = false;

	auto message_type_typed = static_cast<SnakebiteMessageType>(request.message_type());
	switch (message_type_typed)
	{
	case SnakebiteMessageType::InitPlayerSnake:
	{
		InitPlayerSnakeRequest rq;
		rq.ParseFromArray(request.body(), request.body_length());

		sc_messages::InitPlayerSnakeReply rp;
		ret = InitPlayerSnakeMessage(session, rq, &rp);
		{
			reply.reset(new SnakebiteMessage);
			int reply_length = 0;
			rp.SerializeToArray(reply->body(), reply->max_body_length);
			reply->body_length((unsigned short)rp.ByteSize());
			reply->encode_header((unsigned short)SC_SnakebiteMessageType::ReplyInitPlayerSnake);
		}

		break;
	}
	case SnakebiteMessageType::TurnKeyDown:
	{
		TurnKeyDownRequest rq;
		rq.ParseFromArray(request.body(), request.body_length());
		ret = TurnKeyDownMessage(session, rq);

		break;
	}
	case SnakebiteMessageType::TurnKeyUp:
	{
		TurnKeyUpRequest rq;
		rq.ParseFromArray(request.body(), request.body_length());
		ret = TurnKeyUpMessage(session, rq);

		break;
	}
	default:
		break;
	}

	return ret;
}

bool SnakebiteMessageHandlerTable::TurnKeyDownMessage(UserSession& session, messages::TurnKeyDownRequest& rq)
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

bool SnakebiteMessageHandlerTable::TurnKeyUpMessage(UserSession& session, messages::TurnKeyUpRequest& rq)
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

bool SnakebiteMessageHandlerTable::InitPlayerSnakeMessage(UserSession& session, messages::InitPlayerSnakeRequest& rq, sc_messages::InitPlayerSnakeReply* rp)
{
	auto handle = session.RequestInitPlayer(rq.name());

	if (rp)
	{
		rp->set_handle(handle);
	}

	return true;
}

}
}
