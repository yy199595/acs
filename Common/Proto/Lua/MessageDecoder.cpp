//
// Created by yjz on 2022/6/3.
//

#include"Message.h"

#define NOUSER
namespace acs
{
	MessageDecoder::MessageDecoder(lua_State* lua)
	{
		this->mLua = lua;
		this->mMsgComponent = nullptr;
	}
	MessageDecoder::MessageDecoder(lua_State* lua, ProtoComponent* component)
	{
		this->mLua = lua;
		this->mMsgComponent = component;
	}

	bool MessageDecoder::Decode(const pb::Message& message)
	{
		const pb::Descriptor* descriptor = message.GetDescriptor();
		lua_createtable(this->mLua, 0, descriptor->field_count());
		for (int index = 0; index < descriptor->field_count(); index++)
		{
			if(!this->DecodeField(message, descriptor->field(index)))
			{
				return false;
			}
			lua_setfield(this->mLua, -2, descriptor->field(index)->name().c_str());
		}
		return true;
	}

	bool MessageDecoder::DecodeField(const pb::Message& message, const pb::FieldDescriptor* field)
	{
		if (field->is_map())
		{
			return this->DecodeTable(message, field);
		}
		else if (field->is_required())
		{
			return this->DecodeRequired(message, field);
		}
		else if (field->is_optional())
		{
			return this->DecodeOptional(message, field);
		}
		else if (field->is_repeated())
		{
			return this->DecodeRepeted(message, field);
		}
		return false;
	}

	bool MessageDecoder::DecodeTable(const pb::Message& message, const pb::FieldDescriptor* field)
	{
		const pb::Reflection* reflection = message.GetReflection();
		int field_size = reflection->FieldSize(message, field);

		const pb::Descriptor* descriptor = field->message_type();
		if(descriptor->field_count() != 2)
		{
			return false;
		}
		const pb::FieldDescriptor* key = descriptor->field(0);
		const pb::FieldDescriptor* value = descriptor->field(1);

		lua_createtable(this->mLua, 0, field_size);
		for (int index = 0; index < field_size; index++)
		{
			const pb::Message& submessage = reflection->GetRepeatedMessage(message, field, index);
			this->DecodeField(submessage, key);
			this->DecodeField(submessage, value);
			lua_settable(this->mLua, -3);
		}
		return true;
	}

	bool MessageDecoder::DecodeRequired(const pb::Message& message, const pb::FieldDescriptor* field)
	{
		const pb::Reflection* reflection = message.GetReflection();
		if (!reflection->HasField(message, field)) {
			luaL_error(this->mLua, "decode_required field notFound, field=%s", field->full_name().c_str());
			return true;
		}
		return this->DecodeSingle(message, field);
	}

	bool MessageDecoder::DecodeOptional(const pb::Message& message, const pb::FieldDescriptor* field)
	{
		const pb::Reflection* reflection = message.GetReflection();
		if (field->cpp_type() == pb::FieldDescriptor::CPPTYPE_MESSAGE && !reflection->HasField(message, field)) {
			lua_pushnil(this->mLua);
			return true;
		}
		return this->DecodeSingle(message, field);
	}

	bool MessageDecoder::DecodeRepeted(const pb::Message& message, const pb::FieldDescriptor* field)
	{
		const pb::Reflection* reflection = message.GetReflection();
		int field_size = reflection->FieldSize(message, field);
		lua_createtable(this->mLua, field_size, 0);
		for (int index = 0; index < field_size; index++)
		{
			if(!this->DecodeMutiple(message, field,  index))
			{
				return false;
			}
			lua_seti(this->mLua, -2, index + 1);
		}
		return true;
	}

	bool MessageDecoder::DecodeMutiple(const pb::Message& message, const pb::FieldDescriptor* field, int index)
	{
		const pb::Reflection* reflection = message.GetReflection();
		switch (field->cpp_type())
		{
			case pb::FieldDescriptor::CPPTYPE_DOUBLE:
				lua_pushnumber(this->mLua, reflection->GetRepeatedDouble(message, field, index));
				break;
			case pb::FieldDescriptor::CPPTYPE_FLOAT:
				lua_pushnumber(this->mLua, reflection->GetRepeatedFloat(message, field, index));
				break;
			case pb::FieldDescriptor::CPPTYPE_INT32:
				lua_pushinteger(this->mLua, reflection->GetRepeatedInt32(message, field, index));
				break;
			case pb::FieldDescriptor::CPPTYPE_UINT32:
				lua_pushinteger(this->mLua, reflection->GetRepeatedUInt32(message, field, index));
				break;
			case pb::FieldDescriptor::CPPTYPE_INT64:
				lua_pushinteger(this->mLua, reflection->GetRepeatedInt64(message, field, index));
				break;
			case pb::FieldDescriptor::CPPTYPE_UINT64:
				lua_pushinteger(this->mLua, reflection->GetRepeatedUInt64(message, field, index));
				break;
			case pb::FieldDescriptor::CPPTYPE_ENUM:
				lua_pushinteger(this->mLua, reflection->GetRepeatedEnumValue(message, field, index));
				break;
			case pb::FieldDescriptor::CPPTYPE_BOOL:
				lua_pushboolean(this->mLua, reflection->GetRepeatedBool(message, field, index));
				break;
			case pb::FieldDescriptor::CPPTYPE_MESSAGE:
			{
				const pb::Message& msg = reflection->GetRepeatedMessage(message, field, index);
				if (this->mMsgComponent != nullptr && msg.GetTypeName() == "google.protobuf.Any")
				{
					std::unique_ptr<pb::Message> anyMessage;
					const pb::Any& any = dynamic_cast<const pb::Any&>(msg);
					if(!this->mMsgComponent->New(any, anyMessage))
					{
						return false;
					}
					this->Decode(*anyMessage);
					return true;
				}
				this->Decode(msg);
			}
				break;
			case pb::FieldDescriptor::CPPTYPE_STRING:
			{
				std::string value = reflection->GetRepeatedString(message, field, index);
				lua_pushlstring(this->mLua, value.c_str(), value.size());
			}
				break;
			default:
				luaL_error(this->mLua, "decode_multiple field unknow type, field=%s", field->full_name().c_str());
				return false;
		}
		return true;
	}

	bool MessageDecoder::DecodeSingle(const pb::Message& message, const pb::FieldDescriptor* field)
	{
		const pb::Reflection* reflection = message.GetReflection();
		switch (field->cpp_type())
		{
		case pb::FieldDescriptor::CPPTYPE_DOUBLE:
			lua_pushnumber(this->mLua, reflection->GetDouble(message, field));
			break;
		case pb::FieldDescriptor::CPPTYPE_FLOAT:
			lua_pushnumber(this->mLua, reflection->GetFloat(message, field));
			break;
		case pb::FieldDescriptor::CPPTYPE_INT32:
			lua_pushinteger(this->mLua, reflection->GetInt32(message, field));
			break;
		case pb::FieldDescriptor::CPPTYPE_UINT32:
			lua_pushinteger(this->mLua, reflection->GetUInt32(message, field));
			break;
		case pb::FieldDescriptor::CPPTYPE_INT64:
			lua_pushinteger(this->mLua, reflection->GetInt64(message, field));
			break;
		case pb::FieldDescriptor::CPPTYPE_UINT64:
			lua_pushinteger(this->mLua, reflection->GetUInt64(message, field));
			break;
		case pb::FieldDescriptor::CPPTYPE_ENUM:
			lua_pushinteger(this->mLua, reflection->GetEnumValue(message, field));
			break;
		case pb::FieldDescriptor::CPPTYPE_BOOL:
			lua_pushboolean(this->mLua, reflection->GetBool(message, field));
			break;
		case pb::FieldDescriptor::CPPTYPE_MESSAGE:
		{
#ifdef __OS_WIN__
#undef GetMessage //
#endif

			const pb::Message & msg = reflection->GetMessage(message, field);
			if(this->mMsgComponent != nullptr && msg.GetTypeName() == "google.protobuf.Any")
			{
				std::unique_ptr<pb::Message> anyMessage;
				const pb::Any & any = dynamic_cast<const pb::Any&>(msg);
				if(!this->mMsgComponent->New(any, anyMessage))
				{
					return false;
				}
				this->Decode(*anyMessage);
				return true;
			}
			this->Decode(msg);
		}
			break;
		case pb::FieldDescriptor::CPPTYPE_STRING:
		{
			std::string value = reflection->GetString(message, field);
			lua_pushlstring(this->mLua, value.c_str(), value.size());
		}
			break;
		default:
			luaL_error(this->mLua, "decode_single field unknown type, field=%s", field->full_name().c_str());
			return false;
		}
		return true;
	}
}