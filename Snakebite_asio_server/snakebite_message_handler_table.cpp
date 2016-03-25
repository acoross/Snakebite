#include "snakebite_message_handler_table.h"

#include "UserSession.h"
#include <acoross/snakebite/protos/snakebite_message_type.h>
#include <acoross/snakebite/protos/snakebite_message.h>

namespace acoross {
namespace snakebite {

bool SnakebiteMessageHandlerTable::ProcessMessage(UserSession& session, const SnakebiteMessage& request, SnakebiteMessage* reply)
{
	using namespace messages;

	if (reply == nullptr)
	{
		return false;
	}

	if (request.message_type() >= static_cast<unsigned short>(SnakebiteMessageType::Max))
	{
		return false;
	}

	std::unique_ptr<::google::protobuf::Message> rp = nullptr;
	bool ret = false;

	auto message_type_typed = static_cast<SnakebiteMessageType>(request.message_type());
	switch (message_type_typed)
	{
	case SnakebiteMessageType::InitPlayerSnake:
	{
		InitPlayerSnakeRequest rq;
		rq.ParseFromArray(request.body(), request.body_length());

		rp.reset(new VoidReply);
		auto* rpp = (VoidReply*)rp.get();
		ret = InitPlayerSnakeMessage(session, rq);
		rpp->set_err(ret);

		break;
	}
	case SnakebiteMessageType::TurnKeyDown:
	{
		TurnKeyDownRequest rq;
		rq.ParseFromArray(request.body(), request.body_length());

		rp.reset(new VoidReply);
		auto* rpp = (VoidReply*)rp.get();
		ret = TurnKeyDownMessage(session, rq);
		rpp->set_err(ret);

		break;
	}
	case SnakebiteMessageType::TurnKeyUp:
	{
		TurnKeyUpRequest rq;
		rq.ParseFromArray(request.body(), request.body_length());

		rp.reset(new VoidReply);
		auto* rpp = (VoidReply*)rp.get();
		ret = TurnKeyUpMessage(session, rq);
		rpp->set_err(ret);

		break;
	}
	default:
		break;
	}

	if (rp)
	{
		int reply_length = 0;
		rp->SerializeToArray(reply->body(), reply_length);
		reply->body_length((unsigned short)reply_length);
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

bool SnakebiteMessageHandlerTable::InitPlayerSnakeMessage(UserSession& session, messages::InitPlayerSnakeRequest& rq)
{
	session.RequestInitPlayer(rq.name());

	return true;
}

}
}
