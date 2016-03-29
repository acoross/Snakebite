#include "asio_protobuf_rpc_stub.h"
#include <boost/asio.hpp>
#include <acoross/snakebite/>
bool acoross::snakebite::rpc::SnakebiteRpcStub::Invoke(messages::MessageType msgt, const::google::protobuf::Message & msg)
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
