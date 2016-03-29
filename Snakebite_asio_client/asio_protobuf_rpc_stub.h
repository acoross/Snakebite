#ifndef SNAKEBITE_ASIO_PROTOBUF_RPC_STUB_H_
#define SNAKEBITE_ASIO_PROTOBUF_RPC_STUB_H_

#include <acoross/snakebite/protos/snakebite_message_type.h>
#include <acoross/snakebite/protos/snakebite_message.pb.h>

namespace acoross {
namespace snakebite {
namespace rpc {

class RpcMessage;

class SnakebiteRpcStub
{
public:
	bool Connect();
	bool Invoke(messages::MessageType msgt, const ::google::protobuf::Message& msg);
	
private:
	bool send();
};

} //rpc
} //snakebite
} //acoross
#endif //SNAKEBITE_ASIO_PROTOBUF_RPC_STUB_H_