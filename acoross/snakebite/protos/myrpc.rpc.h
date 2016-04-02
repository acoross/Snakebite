#pragma once

#include "myrpc.define.h"

#include <acoross/snakebite/win/targetver.h>
#include <boost/asio.hpp>
#include <acoross/rpc/rpc_service.h>
#include <acoross/rpc/rpc_stub.h>
#include <acoross/snakebite/protos/snakebite_message.pb.h>
#include <acoross/rpc/rpc_macros.h>

namespace acoross {
namespace myrpc {

class MyRpc
{
public:
	enum Protocol
	{
#define DEF_PROCEDURE(name, requestT, replyT) name##_type,
		PROCEDURE_LIST
#undef DEF_PROCEDURE
	};

	class Service : public ::acoross::rpc::RpcService
	{
	public:
		Service(::boost::asio::io_service& io_service, ::boost::asio::ip::tcp::socket&& socket);
		virtual ~Service() {}

	private:
#define DEF_PROCEDURE(name, requestT, replyT)	DEF_SERVICE(name, requestT, replyT)
		PROCEDURE_LIST
#undef DEF_PROCEDURE
	};

	class Stub : public ::acoross::rpc::RpcStub
	{
	public:
		Stub(::boost::asio::io_service& io_service, ::boost::asio::ip::tcp::socket socket)
			: ::acoross::rpc::RpcStub(io_service, std::move(socket))
		{}
		virtual ~Stub() {}

		// member functions
#define DEF_PROCEDURE(name, requestT, replyT)	DEF_STUB(name, requestT, replyT)
		PROCEDURE_LIST
#undef DEF_PROCEDURE
	};
};

}
}