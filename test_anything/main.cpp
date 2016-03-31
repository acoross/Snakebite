#include <iostream>
#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>

#include <acoross/snakebite/win/targetver.h>
#include <boost/asio.hpp>
#include <acoross/rpc/rpc_stub.h>
#include <acoross/rpc/rpc_service.h>
#include <acoross/rpc/rpc_server.h>
#include <acoross/snakebite/protos/snakebite_message.pb.h>

using namespace acoross::snakebite;
using namespace acoross;
using ::boost::asio::ip::tcp;

namespace acoross {
namespace myrpc {

#define DEF_END
#define PROCEDURE_LIST \
	DEF_PROCEDURE(Hello, messages::HelloRequest, messages::HelloReply)	\
	DEF_END

// enum
enum Protocol
{
#define DEF_PROCEDURE(name, requestT, replyT) name##_type,
	PROCEDURE_LIST
#undef DEF_PROCEDURE
};

class MyRpcStub : public rpc::RpcStub
{
public:
	MyRpcStub(::boost::asio::io_service& io_service, tcp::socket socket)
		: rpc::RpcStub(io_service, std::move(socket))
	{}
	virtual ~MyRpcStub() {}

	// member functions
#define DEF_PROCEDURE(name, requestT, replyT)	\
	void name(const requestT& rq, std::function<void(rpc::ErrCode, replyT&)> cb)\
	{	RpcCaller<replyT>((unsigned short)Protocol::name##_type, rq, std::move(cb));	}

	PROCEDURE_LIST

#undef DEF_PROCEDURE
};

class MyRpcService : public rpc::RpcService
{
public:
	MyRpcService(::boost::asio::io_service& io_service, tcp::socket socket)
		: rpc::RpcService(io_service, std::move(socket))
	{
#define DEF_PROCEDURE(name, requestT, replyT)	\
		procedures_[(unsigned short)Protocol::name##_type] = std::make_shared<rpc::ProcedureCaller<requestT, replyT>>(&name);

		PROCEDURE_LIST

#undef DEF_PROCEDURE
	}
	
	virtual ~MyRpcService() {}

private:
#define DEF_PROCEDURE(name, requestT, replyT)	\
	static rpc::ErrCode name(const requestT& rq, replyT* rp);

	PROCEDURE_LIST

#undef DEF_PROCEDURE
};

inline rpc::ErrCode MyRpcService::Hello(const messages::HelloRequest& rq, messages::HelloReply* rp)
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

}
}


using namespace acoross::myrpc;

void send_name(std::shared_ptr<MyRpcStub> rpc_stub, char* name)
{
	messages::HelloRequest rq;
	rq.set_name(name);
	rpc_stub->Hello(rq,
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