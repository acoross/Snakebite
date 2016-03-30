#include "asio_protobuf_rpc_stub.h"

#include <acoross/snakebite/protos/snakebite_message_type.h>
#include <acoross/snakebite/protos/snakebite_message.h>

namespace acoross {
namespace snakebite {
namespace rpc {

bool SnakebiteRpcStub::Invoke(messages::MessageType msg_type, const ::google::protobuf::Message& rq, ::google::protobuf::Message* rp)
{
	SnakebiteMessage msg;

	rq.SerializeToArray(msg.body(), msg.max_body_length);
	msg.body_length(rq.ByteSize());
	msg.encode_header((unsigned short)msg_type);

	::boost::asio::write(socket_, boost::asio::buffer(msg.data(), msg.length()));

	SnakebiteMessage rp_msg;
	::boost::asio::read(socket_, boost::asio::buffer(rp_msg.data(), rp_msg.max_body_length));

}

}
}
}