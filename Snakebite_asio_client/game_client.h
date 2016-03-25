#ifndef SNAKEBITE_GAME_CLIENT_H_
#define SNAKEBITE_GAME_CLIENT_H_

#include <boost/asio.hpp>

#include <acoross/snakebite/protos/snakebite_message.h>
#include <acoross/snakebite/protos/snakebite_message_type.h>
#include <acoross/snakebite/protos/snakebite_message.pb.h>

#include <acoross/snakebite/snake.h>

using boost::asio::ip::tcp;
enum { max_length = 1024 };

namespace acoross {
namespace snakebite {

class GameClient final
{
public:
	GameClient(boost::asio::io_service& io_service)
		: io_service_(io_service)
		, socket_(io_service)
	{}

	void ConnectToServer(char* host, char* port)
	{
		tcp::resolver resolver(io_service_);
		boost::asio::connect(socket_, resolver.resolve({ host, port }));
	}

	void InitPlayer()
	{
		SnakebiteMessage msg;
		auto msg_type = SnakebiteMessageType::InitPlayerSnake;

		messages::InitPlayerSnakeRequest rq;
		rq.set_name("remote player");

		rq.SerializeToArray(msg.body(), msg.max_body_length);
		msg.body_length(rq.ByteSize());
		msg.encode_header((unsigned short)msg_type);

		boost::asio::write(socket_, boost::asio::buffer(msg.data(), msg.length()));
	}

	void TurnKeyDown(PlayerKey pk)
	{
		SnakebiteMessage msg;
		auto msg_type = SnakebiteMessageType::TurnKeyDown;

		messages::TurnKeyDownRequest rq;
		rq.set_key((google::protobuf::int32)pk);

		rq.SerializeToArray(msg.body(), msg.max_body_length);
		msg.body_length(rq.ByteSize());
		msg.encode_header((unsigned short)msg_type);

		boost::asio::write(socket_, boost::asio::buffer(msg.data(), msg.length()));
	}

	void TurnKeyUp(PlayerKey pk)
	{
		SnakebiteMessage msg;
		auto msg_type = SnakebiteMessageType::TurnKeyUp;

		messages::TurnKeyUpRequest rq;
		rq.set_key((google::protobuf::int32)pk);

		rq.SerializeToArray(msg.body(), msg.max_body_length);
		msg.body_length(rq.ByteSize());
		msg.encode_header((unsigned short)msg_type);

		boost::asio::write(socket_, boost::asio::buffer(msg.data(), msg.length()));
	}

private:
	boost::asio::io_service& io_service_;
	tcp::socket socket_;
};

}
}
#endif //SNAKEBITE_GAME_CLIENT_H_