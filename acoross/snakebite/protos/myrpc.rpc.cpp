#include "myrpc.rpc.h"

#include <acoross/snakebite/win/targetver.h>
#include <boost/asio.hpp>

namespace acoross {
namespace myrpc {

MyRpc::Service::Service(::boost::asio::io_service & io_service, ::boost::asio::ip::tcp::socket && socket)
	: acoross::rpc::RpcService(io_service, std::move(socket))
{
#define DEF_PROCEDURE(name, requestT, replyT)	REGISTER_SERVICE(name, requestT, replyT)
	PROCEDURE_LIST
#undef DEF_PROCEDURE
}

}
}
