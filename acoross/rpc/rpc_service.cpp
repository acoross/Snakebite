#include "rpc_service.h"
#include <acoross/snakebite/protos/snakebite_message.pb.h>

namespace acoross {
namespace rpc {

bool RpcService::process_msg(RpcPacket& request_packet)
{
	auto rpc_msg_uid = request_packet.get_header().rpc_msg_uid_;
	auto err_code = request_packet.get_header().error_code_;

	auto it = procedures_.find(request_packet.message_type());
	if (it == procedures_.end())
	{
		return false;
	}
	
	auto procedure = it->second;
	auto reply_packet = std::make_shared<RpcPacket>();
	bool ret = (*procedure)(request_packet, reply_packet);
	if (ret == false)
	{
		return false;
	}

	send(reply_packet);
	return true;
}

}
}
