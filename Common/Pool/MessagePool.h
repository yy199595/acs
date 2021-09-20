#pragma once
#include<queue>
#include<google/protobuf/message.h>
#include<google/protobuf/util/json_util.h>
using namespace google::protobuf;
using namespace google::protobuf::util;
namespace Sentry
{
	class MessagePool
	{
	public:
		static Message * New(const std::string & name);
		static Message * NewByJson(const std::string & name, const std::string & json);
		static Message * NewByJson(const std::string & name, const char * json, size_t size);
	public:
		static Message * NewByData(const std::string & name, const std::string & data);
		static Message * NewByData(const std::string & name, const char * json, size_t size);
	private:
		static std::unordered_map<std::string, Message *> mMessageMap;
	};
}