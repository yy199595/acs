//
// Created by zmhy0073 on 2022/8/29.
//

#include"ProtoHelper.h"
#include"Proto/Include/Message.h"
namespace Helper
{
    bool Protocol::FromJson(pb::Message *message, const std::string &json)
    {
        if(message == nullptr || json.empty())
        {
            return false;
        }
        if(!pb_json::JsonStringToMessage(json, message).ok())
        {
            return false;
        }
        return true;
    }

    bool Protocol::GetJson(const pb::Message &message, std::string *json)
    {
        return pb_json::MessageToJsonString(message, json).ok();
    }

    bool Protocol::GetMember(const std::string & key, const pb::Message & message, std::string &value)
    {
        const pb::Reflection *reflection = message.GetReflection();
        const pb::Descriptor *descriptor = message.GetDescriptor();
        const pb::FieldDescriptor *fileDesc = descriptor->FindFieldByName(key);
        if(fileDesc == nullptr || fileDesc->is_repeated())
        {
            return false;
        }
        switch (fileDesc->cpp_type())
        {
            case pb::FieldDescriptor::CppType::CPPTYPE_INT32:
                value = std::to_string(reflection->GetInt64(message, fileDesc));
                break;
        case pb::FieldDescriptor::CppType::CPPTYPE_UINT32:
                value = std::to_string(reflection->GetUInt32(message, fileDesc));
                break;
        case pb::FieldDescriptor::CppType::CPPTYPE_INT64:
                value = std::to_string(reflection->GetInt64(message, fileDesc));
                break;
        case pb::FieldDescriptor::CppType::CPPTYPE_UINT64:
                value = std::to_string(reflection->GetUInt64(message, fileDesc));
                break;
        case pb::FieldDescriptor::CppType::CPPTYPE_FLOAT:
                value = std::to_string(reflection->GetFloat(message, fileDesc));
                break;
        case pb::FieldDescriptor::CppType::CPPTYPE_DOUBLE:
                value = std::to_string(reflection->GetDouble(message, fileDesc));
                break;
        case pb::FieldDescriptor::CppType::CPPTYPE_STRING:
                value = reflection->GetString(message, fileDesc);
                break;
			default:
				return false;
        }
        return true;
    }
}