#include <iostream>
#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>

#include <acoross/snakebite/win/targetver.h>
#include <boost/asio.hpp>

#include <acoross/rpc/rpc_server.h>
#include <acoross/rpc/rpc_macros.h>

#include <acoross/snakebite/protos/test_rpc.rpc.h>

using namespace acoross::rpc_test;
using namespace acoross;
using ::boost::asio::ip::tcp;

namespace acoross {
namespace myrpc {

class MyRpcImpl final
	: public TestRpc::Service
{
public:
	MyRpcImpl(::boost::asio::io_service& io_service, ::boost::asio::ip::tcp::socket&& socket)
		: Service(io_service, std::move(socket))
	{}

	virtual rpc::ErrCode Hello(const HelloRequest& rq, HelloReply* rp) override
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
};

}
}

using namespace acoross::myrpc;

void send_name(std::shared_ptr<TestRpc::Stub> rpc_stub, char* name)
{
	HelloRequest rq;
	rq.set_name(name);
	rpc_stub->Hello(rq,
		[rpc_stub](rpc::ErrCode err_code, HelloReply& rp)
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
			boost::asio::connect(socket, resolver.resolve({ "127.0.0.1", "22001" }));
		}

		auto rpc_stub = std::make_shared<TestRpc::Stub>(io_service, std::move(socket));
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
			std::make_shared<MyRpcImpl>(io_service, std::move(socket))->start();
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