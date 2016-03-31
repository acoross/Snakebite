#ifndef ACOROSS_RPC_SERVICE_H_
#define ACOROSS_RPC_SERVICE_H_

#include <acoross/rpc/rpc_packet.h>

namespace acoross {
namespace rpc {

class RpcService
{
public:
	void start(const char* port);

protected:
	// rpc service procedures declared by macro here. (in child class)

private:

};

}
}
#endif //ACOROSS_RPC_SERVICE_H_