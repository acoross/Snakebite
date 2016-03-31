#ifndef ACOROSS_RPC_MESSAGE_H_
#define ACOROSS_RPC_MESSAGE_H_

namespace acoross {
namespace rpc {

enum class ErrCode
{
	NoError = 0
};

class RpcPacket
{
public:
	struct header
	{
		unsigned short body_length_{ 0 };
		unsigned short message_type_{ 0 };
		ErrCode error_code_{ ErrCode::NoError }; //0:success
		size_t rpc_msg_uid_{ 0 };
	};

	enum { header_length = sizeof(header) };
	enum { max_body_length = 1024 * 32 };

	RpcPacket()
		: header_{}
	{}

	const char* data() const
	{
		return data_;
	}

	char* data()
	{
		return data_;
	}

	unsigned short length() const
	{
		return header_length + header_.body_length_;
	}

	const char* body() const
	{
		return data_ + header_length;
	}

	char* body()
	{
		return data_ + header_length;
	}

	unsigned short body_length() const
	{
		return header_.body_length_;
	}
	
	bool decode_header()
	{
		// Ã¹ 2byte ´Â body length;
		header_ = *reinterpret_cast<header*>(data_);
		if (header_.body_length_ > max_body_length)
		{
			header_.body_length_ = 0;
			return false;
		}
		return true;
	}

	void encode_header(unsigned short message_type, unsigned short new_length, size_t rpc_msg_uid)
	{
		header_.body_length_ = new_length;
		header_.message_type_ = message_type;
		header_.error_code_ = ErrCode::NoError;
		header_.rpc_msg_uid_ = rpc_msg_uid;
		
		if (header_.body_length_ > max_body_length)
			header_.body_length_ = max_body_length;

		*(header*)(&data_[0]) = header_;
	}

	unsigned short message_type() const
	{
		return header_.message_type_;
	}

	header& get_header()
	{
		return header_;
	}

private:
	char data_[header_length + max_body_length]{ '\0' };
	header header_;
};

}
}
#endif //SNAKEBITE_RPC_MESSAGE_H_