#pragma once
#include <google/protobuf/message.h>
using namespace google::protobuf;
namespace ProtocHelper
{
    extern bool GetJsonString(const Message &message, std::string &jsonString);
    extern bool GetJsonString(const Message *message, std::string &jsonString);
    extern bool GetProtocObject(const std::string &jsonString, Message &message);

    extern Message *Create(const std::string name);
    extern Message *Create(const std::string name, const std::string &jsonString);

}// namespace ProtocHelper