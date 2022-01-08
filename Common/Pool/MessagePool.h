#pragma once
#include<queue>
#include"Define/ThreadQueue.h"
#include<google/protobuf/any.pb.h>
#include<google/protobuf/message.h>
#include<google/protobuf/util/json_util.h>
using namespace google::protobuf;
using namespace google::protobuf::util;
namespace Helper
{
	class Proto
	{
	public:
		static Message * New(const Any & any);
		static Message * New(const std::string & name);
        static Message * NewByData(const Any & any,bool clone = false);
        static Message * NewByJson(const Any & any, const std::string & json,bool clone = false);
        static Message * NewByJson(const std::string & name, const std::string & json,bool clone = false);
		static Message * NewByJson(const std::string & name, const char * json, size_t size,bool clone = false);

    public:
        static bool GetJson(const Any & message, std::string & json);
        static bool GetJson(const Message & message, std::string & json);
	public:
		static Message * NewByData(const std::string & name, const std::string & data,bool clone = false);
		static Message * NewByData(const std::string & name, const char * json, size_t size,bool clone = false);
	private:
		static std::unordered_map<std::string, Message *> mMessageMap;
	};
}
