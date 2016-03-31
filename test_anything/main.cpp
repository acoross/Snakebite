#include <iostream>
#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>

#include <acoross/snakebite/win/targetver.h>
#include <boost/asio.hpp>

#include "myrpc.stub.h"
#include "myrpc.service.h"
#include <acoross/rpc/rpc_server.h>
#include <acoross/snakebite/protos/snakebite_message.pb.h>
#include <acoross/rpc/rpc_macros.h>

using namespace acoross::snakebite;
using namespace acoross;
using ::boost::asio::ip::tcp;

namespace acoross {
namespace myrpc {

inline DEF_SERVICE_IMPL(MyRpcService, Hello, messages::HelloRequest, &rq, messages::HelloReply, *rp)
{
	std::cout << "request: " << rq.name() << std::endl;

	if (rp)
	{
		std::string reply_str("fuck you ");
		reply_str += rq.name();
		rp->set_message(reply_str.c_str());
	}

	return rpc::ErrCode::NoError;
}

inline DEF_SERVICE_IMPL(MyRpcService, Hello2, messages::HelloRequest, &rq, messages::HelloReply, *rp)
{
	std::cout << "request: " << rq.name() << std::endl;

	if (rp)
	{
		std::string reply_str("hi, ");
		reply_str += rq.name();
		rp->set_message(reply_str.c_str());
	}

	return rpc::ErrCode::NoError;
}

}
}

using namespace acoross::myrpc;

void send_name(std::shared_ptr<MyRpcStub> rpc_stub, char* name)
{
	messages::HelloRequest rq;
	rq.set_name(name);
	rpc_stub->Hello2(rq,
		[rpc_stub](rpc::ErrCode err_code, messages::HelloReply& rp)
	{
		if (err_code == rpc::ErrCode::NoError)
		{
			std::cout << "reply: " << rp.message() << std::endl;

			char buf[1000] = { 0, };
			std::cout << "name? : ";
			std::cin >> buf;
			send_name(rpc_stub, buf);
		}
	});
}

void stub_thread_func()
{
	try
	{
		::boost::asio::io_service io_service;

		tcp::socket socket(io_service);
		{
			tcp::resolver resolver(io_service);
			boost::asio::connect(socket, resolver.resolve({ "localhost", "22001" }));
		}

		auto rpc_stub = std::make_shared<MyRpcStub>(io_service, std::move(socket));
		rpc_stub->start();

		send_name(rpc_stub, "shin");

		io_service.run();
	}
	catch (std::exception ex)
	{
		std::cout << "exception: " << ex.what() << std::endl;
	}
}

std::condition_variable server_start_cv;
std::mutex server_start_lock;

void service_thread_func()
{
	try
	{
		::boost::asio::io_service io_service;
		
		rpc::RpcServer server(io_service, 22001, 
			[](::boost::asio::io_service& io_service, tcp::socket&& socket)
		{
			std::make_shared<MyRpcService>(io_service, std::move(socket))->start();
		});

		server_start_cv.notify_all();

		io_service.run();
	}
	catch (std::exception ex)
	{
		std::cout << "exception: " << ex.what() << std::endl;
	}
}

int main()
{	
	std::thread service_thread(service_thread_func);

	server_start_cv.wait(std::unique_lock<std::mutex>(server_start_lock));
	stub_thread_func();
	service_thread.join();

	return 0;
}