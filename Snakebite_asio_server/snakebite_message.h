#ifndef SNAKEBITET_SNAKEBITE_MESSAGE_H_
#define SNAKEBITET_SNAKEBITE_MESSAGE_H_

#include <strsafe.h>

namespace acoross {
namespace snakebite {

class SnakebiteMessage
{
public:
	enum { header_length = 4 };
	enum { max_body_length = 512 };

	SnakebiteMessage()
		: body_length_(0)
	{
	}

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
		return header_length + body_length_;
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
		return body_length_;
	}

	void body_length(unsigned short new_length)
	{
		body_length_ = new_length;
		if (body_length_ > max_body_length)
			body_length_ = max_body_length;
	}

	bool decode_header()
	{
		// Ã¹ 2byte ´Â body length;
		unsigned short body_length_ = *reinterpret_cast<unsigned short*>(data_);
		if (body_length_ > max_body_length)
		{
			body_length_ = 0;
			return false;
		}

		message_type_ = *reinterpret_cast<unsigned short*>(data_ + 2);
		return true;
	}

	void encode_header(unsigned short message_type)
	{
		message_type_ = message_type;

		*(unsigned short*)(&data_[0]) = body_length_;
		*(unsigned short*)(&data_[2]) = message_type_;
	}

	unsigned short message_type() const
	{
		return message_type_;
	}

private:
	char data_[header_length + max_body_length]{ '\0' };
	unsigned short body_length_{ 0 };
	unsigned short message_type_{ 0 };
};

}
}
#endif //SNAKEBITET_SNAKEBITE_MESSAGE_H_