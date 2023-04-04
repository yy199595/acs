//
// Created by zmhy0073 on 2022/8/29.
//

#include "ProtoHelper.h"
#include"google/protobuf/util/json_util.h"
namespace Helper
{
    bool Protocol::FromJson(Message *message, const std::string &json)
    {
        if(message == nullptr || json.empty())
        {
            return false;
        }
        if(!util::JsonStringToMessage(json, message).ok())
        {
            return false;
        }
        return true;
    }

    bool Protocol::GetJson(const Message &message, std::string *json)
    {
        return util::MessageToJsonString(message, json).ok();
    }

    bool Protocol::GetMember(const std::string & key, const Message & message, std::string &value)
    {
        const Reflection *reflection = message.GetReflection();
        const Descriptor *descriptor = message.GetDescriptor();
        const FieldDescriptor *fileDesc = descriptor->FindFieldByName(key);
        if(fileDesc == nullptr || fileDesc->is_repeated())
        {
            return false;
        }
        switch (fileDesc->cpp_type())
        {
            case FieldDescriptor::TYPE_INT32:
                value = std::to_string(reflection->GetInt64(message, fileDesc));
                break;
            case FieldDescriptor::TYPE_UINT32:
                value = std::to_string(reflection->GetUInt32(message, fileDesc));
                break;
            case FieldDescriptor::TYPE_INT64:
                value = std::to_string(reflection->GetInt64(message, fileDesc));
                break;
            case FieldDescriptor::TYPE_UINT64:
                value = std::to_string(reflection->GetUInt64(message, fileDesc));
                break;
            case FieldDescriptor::TYPE_FLOAT:
                value = std::to_string(reflection->GetFloat(message, fileDesc));
                break;
            case FieldDescriptor::TYPE_DOUBLE:
                value = std::to_string(reflection->GetDouble(message, fileDesc));
                break;
            case FieldDescriptor::TYPE_STRING:
                value = reflection->GetString(message, fileDesc);
                break;
			default:
				return false;
        }
        return true;
    }
}