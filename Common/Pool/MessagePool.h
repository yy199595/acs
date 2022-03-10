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
		static std::shared_ptr<Message> New(const Any & any);
		static std::shared_ptr<Message> New(const std::string & name);
        static std::shared_ptr<Message> NewByData(const Any & any);
        static std::shared_ptr<Message> NewByJson(const Any & any, const std::string & json);
        static std::shared_ptr<Message> NewByJson(const std::string & name, const std::string & json);
		static std::shared_ptr<Message> NewByJson(const std::string & name, const char * json, size_t size);
    public:
        static bool GetJson(const Any & message, std::string & json);
        static bool GetJson(const Message & message, std::string & json);
		static bool GetJson(std::shared_ptr<Message> message, std::string & json);
	 private:
		static std::unordered_map<std::string, const Message *> mMessageMap;
	};
}
