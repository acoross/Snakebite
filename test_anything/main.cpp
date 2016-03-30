#include <iostream>
#include <acoross/snakebite/win/targetver.h>
#include <boost/asio.hpp>
#include <acoross/snakebite/rpc/asio_protobuf_rpc_stub.h>

using namespace acoross::snakebite;

int main()
{
	::boost::asio::io_service io_service;
	rpc::RpcStub rpc_stub(io_service);
	rpc_stub.Connect("localhost", "22001");
	

	return 0;
}