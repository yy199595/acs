//
// Created by yjz on 2022/5/18.
//

#include"ProtoMessage.h"
#include"Network/Rpc.h"
namespace Tcp
{
	void ProtoMessage::Write(std::ostream& os, char cc)
	{
		os.write(&cc, 1);
	}

	void ProtoMessage::Write(std::ostream& os, int value)
	{
		char buffer[sizeof(int)] = { 0 };
		buffer[0] = value & 0xff;
		buffer[1] = (value >> 8) & 0xff;
		buffer[2] = (value >> 16) & 0xff;
		buffer[3] = (value >> 24) & 0xff;
		//memcpy(buffer, &value, sizeof(int));
		os.write(buffer, sizeof(int));
	}

	void ProtoMessage::Write(std::ostream& os, const std::string& value)
	{
		os.write(value.c_str(), value.size());
	}

	void ProtoMessage::Write(std::ostream& os, const char* str, size_t size)
	{
		os.write(str, size);
	}
}