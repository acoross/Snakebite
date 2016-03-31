#pragma once

#include "myrpc.define.h"

#include <acoross/snakebite/win/targetver.h>
#include <boost/asio.hpp>
#include <acoross/rpc/rpc_stub.h>
#include <acoross/snakebite/protos/snakebite_message.pb.h>
#include <acoross/rpc/rpc_macros.h>

namespace acoross {
namespace myrpc {

class MyRpcStub : public rpc::RpcStub
{
public:
	enum Protocol
	{
#define DEF_PROCEDURE(name, requestT, replyT) name##_type,
		PROCEDURE_LIST
#undef DEF_PROCEDURE
	};

	MyRpcStub(::boost::asio::io_service& io_service, ::boost::asio::ip::tcp::socket socket)
		: rpc::RpcStub(io_service, std::move(socket))
	{}
	virtual ~MyRpcStub() {}

	// member functions
#define DEF_PROCEDURE(name, requestT, replyT)	DEF_STUB(name, requestT, replyT)
	PROCEDURE_LIST
#undef DEF_PROCEDURE
};

}
}