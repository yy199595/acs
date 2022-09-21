//
// Created by yjz on 2022/5/18.
//

#ifndef _PROTOMESSAGE_H_
#define _PROTOMESSAGE_H_
#include<ostream>
#include<Message/com.pb.h>

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
}
#endif //_PROTOMESSAGE_H_
