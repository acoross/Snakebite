#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include <SDKDDKVer.h>
#include <boost/asio.hpp>

#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#include <boost/thread/future.hpp>

#include <acoross/rpc/rpc_macros.h>
#include <acoross/rpc/rpc_server.h>
#include <acoross/snakebite/protos/test_rpc.rpc.h>

#include <acoross/snakebite/zone_system/details/signal.h>

#include <boost/signals2.hpp>

using namespace acoross::rpc_test;
using namespace acoross;
using ::boost::asio::ip::tcp;

namespace acoross {
namespace myrpc {
class MyRpcImpl final : public TestRpc::Service
{
public:
	MyRpcImpl(::boost::asio::io_service &io_service,
		::boost::asio::ip::tcp::socket &&socket)
		: Service(io_service, std::move(socket))
	{}

	virtual rpc::ErrCode Hello(const HelloRequest &rq, HelloReply *rp) override
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

boost::future<HelloReply> send_name(std::shared_ptr<TestRpc::Stub> rpc_stub,
	char *name)
{
	auto prom = std::make_shared<boost::promise<HelloReply>>();
	auto fut = prom->get_future();

	HelloRequest rq;
	rq.set_name(name);
	rpc_stub->Hello(rq, [prom, rpc_stub](rpc::ErrCode err_code, HelloReply &rp)
	{
		if (err_code == rpc::ErrCode::NoError)
		{
			prom->set_value(rp);
		}
	});

	return fut;
}

void loop_get_name_and_send(std::shared_ptr<TestRpc::Stub> rpc_stub)
{
	char buf[1000] =
	{
		0,
	};
	std::cout << "name? : ";
	std::cin >> buf;

	send_name(rpc_stub, buf).then([rpc_stub](boost::future<HelloReply> rp)
	{
		auto dt = rp.get();
		std::cout << "reply: " << dt.message() << std::endl;

		loop_get_name_and_send(rpc_stub);
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

		auto rpc_stub =
			std::make_shared<TestRpc::Stub>(io_service, std::move(socket));
		rpc_stub->start();

		send_name(rpc_stub, "shin").then([rpc_stub](boost::future<HelloReply> fut)
		{
			auto rp = fut.get();
			std::cout << "reply: " << rp.message() << std::endl;

			loop_get_name_and_send(rpc_stub);
		});

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

		rpc::RpcServer server(
			io_service, 22001,
			[](::boost::asio::io_service &io_service, tcp::socket &&socket)
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

void future_test()
{
	auto prom = boost::promise<int>();
	boost::shared_future<int> fut1 = prom.get_future();
	auto fut2 = fut1;

	boost::thread([&prom]()
	{
		prom.set_value(10);
	});

	std::cout << "main tid: " << std::this_thread::get_id() << std::endl;
	fut1.then(boost::launch::deferred,
		[](auto i)
	{
		std::cout << "tid: " << std::this_thread::get_id() << std::endl;
		std::cout << "fut1: " << i.get() << std::endl;
	}).then(boost::launch::deferred,
		[](boost::future<void> f)
	{
		f.get();
		std::cout << "tid: " << std::this_thread::get_id() << std::endl;
	})
		.then([](boost::future<void> f)
	{
		f.get();
		std::cout << "tid: " << std::this_thread::get_id() << std::endl;
	})
		.wait();

	/*fut2.then(boost::launch::deferred, [](auto i)
	{
	std::cout << "tid: " << std::this_thread::get_id() << std::endl;
	std::cout << "fut2: " << i.get() << std::endl;
	});*/

	// fut2.wait();
	//
}

void test_my_event()
{
	acoross::Event<void(int)> ev;
	ev.connect([](int a)
	{
		std::cout << "con1 ";
		std::cout << a << std::endl;
	});
	auto auto_conn = ev.auto_connect([](int b)
	{
		std::cout << "con2 ";
		std::cout << b << std::endl;
	});
	ev.invoke(10);
	auto_conn.reset(nullptr);

	//
	auto ev2(std::move(ev));
	std::cout << "ev2" << std::endl;
	ev2.invoke(2);

	//
	auto relayer = ev2.make_relayer_up();
	auto auto_con3 = relayer->auto_connect([](int c)
	{
		std::cout << "con3 ";
		std::cout << c << std::endl;
	});
	auto auto_con4 = relayer->auto_connect([](int c)
	{
		std::cout << "con4 ";
		std::cout << c << std::endl;
	});
	ev2.invoke(3);
}

int main()
{
	test_my_event();

	system("pause");
	return 0;

	{
		std::cout << "size: " << sizeof(acoross::auto_connection) << std::endl;
		std::cout << "main tid: " << std::this_thread::get_id() << std::endl;

		{
			acoross::auto_connection c;
			if (c)
			{
				bool b = c->connected();
			}
		}

		boost::signals2::signal<void(int)> sig;
		acoross::auto_connection con(acoross::make_auto_con(sig.connect([](int i)
		{
			std::cout << i << std::endl;
			std::cout << "tid: " << std::this_thread::get_id() << std::endl;
		})));

		con.swap(acoross::auto_connection());

		auto con2 = sig.connect(
			[](int i)
		{
			std::cout << "connected: " << i << std::endl;
		});

		auto fut = std::async(std::launch::async, [&sig]()
		{
			std::cout << "async tid: " << std::this_thread::get_id() << std::endl;
			sig(1);
			return 0;
		});

		std::cout << fut.get() << std::endl;
	}

	system("pause");

	std::thread service_thread(service_thread_func);

	server_start_cv.wait(std::unique_lock<std::mutex>(server_start_lock));

	std::thread client_thread(stub_thread_func);

	client_thread.join();
	service_thread.join();
	return 0;
}