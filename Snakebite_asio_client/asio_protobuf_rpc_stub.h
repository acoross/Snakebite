#ifndef SNAKEBITE_ASIO_PROTOBUF_RPC_STUB_H_
#define SNAKEBITE_ASIO_PROTOBUF_RPC_STUB_H_

#include <boost/asio.hpp>

#include <acoross/snakebite/protos/snakebite_message_type.h>
#include <acoross/snakebite/protos/snakebite_message.pb.h>

namespace acoross {
namespace snakebite {
namespace rpc {

using boost::asio::ip::tcp;

class SnakebiteRpcStub
{
public:
	SnakebiteRpcStub(::boost::asio::io_service& io_service);
	virtual ~SnakebiteRpcStub();

	bool Connect();
	bool Invoke(messages::MessageType msg_type, const ::google::protobuf::Message& rq, ::google::protobuf::Message* rp);
	
private:
	tcp::socket socket_;	//rpc 전용 sync 소켓. 하나의 write 에 대해 하나의 read 가능
};

} //rpc
} //snakebite
} //acoross
#endif //SNAKEBITE_ASIO_PROTOBUF_RPC_STUB_H_