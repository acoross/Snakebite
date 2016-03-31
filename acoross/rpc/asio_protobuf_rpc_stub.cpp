#include "asio_protobuf_rpc_stub.h"

namespace acoross {
namespace rpc {

bool RpcStub::Connect(char* host, char* port)
{
	tcp::resolver resolver(io_service_);
	boost::asio::connect(socket_, resolver.resolve({ host, port }));

	return false;
}

void RpcStub::AsyncInvoke(unsigned short msg_type, const ::google::protobuf::Message& rq, ReplyCallbackF&& cb)
{
	auto msg = std::make_shared<RpcPacket>();

	rq.SerializeToArray(msg->body(), msg->max_body_length);
	auto rpc_msg_uid = RegisterReplyCallback(std::move(cb));
	msg->encode_header(msg_type, rq.ByteSize(), rpc_msg_uid);

	send(msg);
}

void RpcStub::send(std::shared_ptr<RpcPacket> new_msg)
{
	bool write_in_progress = !write_msgs_.empty();
	write_msgs_.push_back(new_msg);
	if (write_in_progress)
	{
		do_write();
	}
}

void RpcStub::do_write()
{
	auto self(shared_from_this());
	boost::asio::async_write(socket_,
		boost::asio::buffer(write_msgs_.front()->data(),
			write_msgs_.front()->length()),
		[this, self](boost::system::error_code ec, std::size_t /*length*/)
	{
		if (!ec)
		{
			write_msgs_.pop_front();
			if (!write_msgs_.empty())
			{
				do_write();
			}
		}
		else
		{
			end();
		}
	});
}

void RpcStub::do_read_header()
{
	auto self(shared_from_this());
	boost::asio::async_read(socket_,
		boost::asio::buffer(read_msg_->data(), RpcPacket::header_length),
		[this, self](boost::system::error_code ec, std::size_t /*length*/)
	{
		if (!ec && read_msg_->decode_header())
		{
			do_read_body();
		}
		else
		{
			end();
		}
	});
}

void RpcStub::do_read_body()
{
	auto self(shared_from_this());
	boost::asio::async_read(socket_,
		boost::asio::buffer(read_msg_->body(), read_msg_->body_length()),
		[this, self](boost::system::error_code ec, std::size_t /*length*/)
	{
		if (!ec && process_reply(read_msg_))
		{
			do_read_header();
		}
		else
		{
			end();
		}
	});
}

size_t RpcStub::RegisterReplyCallback(ReplyCallbackF&& cb)
{
	auto uid = ++rpc_message_uid_;
	wait_reply_queue_[uid] = cb;

	return uid;
}

bool RpcStub::process_reply(std::shared_ptr<RpcPacket> msg)
{
	auto rpc_msg_uid = msg->get_header().rpc_msg_uid_;	
	auto err_code = msg->get_header().error_code_;

	auto it = wait_reply_queue_.find(rpc_msg_uid);
	if (it != wait_reply_queue_.end())
	{
		auto callback = std::move(it->second);

		wait_reply_queue_.erase(it);
		callback(err_code, *msg.get());
	}

	return false;
}

}
}