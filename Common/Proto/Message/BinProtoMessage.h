//
// Created by mac on 2022/6/24.
//

#ifndef SERVER_BINPROTOMESSAGE_H
#define SERVER_BINPROTOMESSAGE_H

#include"ProtoMessage.h"
namespace Tcp
{
	class BinProtoMessage : public ProtoMessage
	{
	public:
		BinProtoMessage(const std::string& str);
		BinProtoMessage(const char* str, size_t size);
		~BinProtoMessage() { delete [] this->mBuffer;}
	public:
		int Serialize(std::ostream& os); //返回剩余要发送的字节数
	private:
		size_t mSize;
		char* mBuffer;
	};
}


#endif //SERVER_BINPROTOMESSAGE_H
