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
		memcpy(buffer, &value, sizeof(int));
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