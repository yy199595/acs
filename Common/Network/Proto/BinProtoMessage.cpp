//
// Created by mac on 2022/6/24.
//

#include "BinProtoMessage.h"

namespace Tcp
{
	BinProtoMessage::BinProtoMessage(const std::string& str)
	{
		this->mSize = str.size();
		this->mBuffer = new char[this->mSize];
		memcpy(this->mBuffer, str.c_str(), this->mSize);
	}

	BinProtoMessage::BinProtoMessage(const char* str, size_t size)
	{
		this->mSize = size;
		memcpy(this->mBuffer, str, this->mSize);
	}

	int BinProtoMessage::Serailize(std::ostream& os)
	{
		os.write(this->mBuffer, this->mSize);
	}
}