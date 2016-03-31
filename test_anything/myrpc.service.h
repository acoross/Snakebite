#pragma once

#include "myrpc.define.h"

#include <acoross/snakebite/win/targetver.h>
#include <boost/asio.hpp>
#include <acoross/rpc/rpc_service.h>
#include <acoross/snakebite/protos/snakebite_message.pb.h>
#include <acoross/rpc/rpc_macros.h>

namespace acoross {
namespace myrpc {

class MyRpcService : public rpc::RpcService
{
public:
	enum Protocol
	{
#define DEF_PROCEDURE(name, requestT, replyT) name##_type,
		PROCEDURE_LIST
#undef DEF_PROCEDURE
	};

	MyRpcService(::boost::asio::io_service& io_service, ::boost::asio::ip::tcp::socket socket)
		: rpc::RpcService(io_service, std::move(socket))
	{
#define DEF_PROCEDURE(name, requestT, replyT)	REGISTER_SERVICE(name, requestT, replyT)
		PROCEDURE_LIST
#undef DEF_PROCEDURE
	}

	virtual ~MyRpcService() {}

private:
#define DEF_PROCEDURE(name, requestT, replyT)	DEF_SERVICE(name, requestT, replyT)
	PROCEDURE_LIST
#undef DEF_PROCEDURE
};

}
}