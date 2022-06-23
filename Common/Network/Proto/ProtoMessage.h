//
// Created by yjz on 2022/5/18.
//

#ifndef _PROTOMESSAGE_H_
#define _PROTOMESSAGE_H_
#include<ostream>
#include<Protocol/com.pb.h>

namespace Tcp
{
	class ProtoMessage
	{
	 public:
		ProtoMessage() = default;
		virtual int Serailize(std::ostream & os) = 0; //返回剩余要发送的字节数
	 protected:
		void Write(std::ostream & os, char cc);
		void Write(std::ostream & os, int value);
		void Write(std::ostream & os, const std::string & value);
		void Write(std::ostream & os, const char * str, size_t size);
	};

	class BinProtoMessage : public ProtoMessage
	{
	public:
		BinProtoMessage(const std::string & str);
		BinProtoMessage(const char * str, size_t size);
		~BinProtoMessage();
	public:
		int Serailize(std::ostream & os); //返回剩余要发送的字节数
	private:
		size_t size;
		char * mBuffer;
	};
	BinProtoMessage::BinProtoMessage(const std::string& str)
	{
		this->mBuffer = new char[str.size()];
		memcpy(this->mBuffer, str.c_str(), str.size());
	}

	BinProtoMessage::~BinProtoMessage()
	{
		delete []mBuffer;
	}

	BinProtoMessage::BinProtoMessage(const char* str, size_t size)
	{
		memcpy(this->mBuffer, str, size);
	}

	int BinProtoMessage::Serailize(std::ostream& os)
	{
		os.write(this->mBuffer, size);
		return 0;
	}

}
#endif //_PROTOMESSAGE_H_
