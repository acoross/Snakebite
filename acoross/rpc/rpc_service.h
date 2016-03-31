#ifndef ACOROSS_RPC_SERVICE_H_
#define ACOROSS_RPC_SERVICE_H_

#include <acoross/snakebite/win/targetver.h>
#include <boost/asio.hpp>
#include <memory>

#include <acoross/rpc/rpc_packet.h>
#include <acoross/rpc/rpc_socket.h>

namespace acoross {
namespace rpc {

class RpcService
	: public RpcSocket
{
public:
	RpcService(::boost::asio::io_service& io_service);
	void start(const char* port);

protected:
	// rpc service procedures declared by macro here. (in child class)

private:

};

}
}
#endif //ACOROSS_RPC_SERVICE_H_