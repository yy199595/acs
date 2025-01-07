//
// Created by yjz on 2022/6/3.
//
#include"Message.h"
#include "Lua/Engine/LuaInclude.h"

namespace acs
{
	MessageEncoder::MessageEncoder(lua_State* lua)
		: mLua(lua)
	{
	}

	bool MessageEncoder::Encode(pb::Message & message, int index)
	{
		if(!lua_istable(this->mLua, index))
		{
			return false;
		}
		
		index = lua_absindex(this->mLua, index);
		const pb::Descriptor* descriptor = message.GetDescriptor();
		return this->EncoddeMessage(message, descriptor, index);
	}

	bool MessageEncoder::EncoddeMessage(pb::Message& message, const pb::Descriptor* descriptor, int index)
	{
		if (!lua_istable(this->mLua, index)) {
			luaL_error(this->mLua, "encode_message field isn't a table, field=%s", descriptor->full_name().c_str());
			return false;
		}
		for (int i = 0; i < descriptor->field_count(); i++)
		{
			const pb::FieldDescriptor* field = descriptor->field(i);
			const std::string & name = field->name();
			lua_getfield(this->mLua, index, field->name().c_str());
			if(!this->EncodeField(message, field, lua_absindex(this->mLua, -1)))
			{
				luaL_error(this->mLua, "encode field : %s", field->name().c_str());
				return false;
			}
			lua_pop(this->mLua, 1);
		}
		return true;
	}

	bool MessageEncoder::EncodeField(pb::Message& message, const pb::FieldDescriptor* field, int index)
	{
		if (field->is_map()){
			return this->EncodeTable(message, field, index);
		}
		if (field->is_required()){
			return this->EncodeRequired(message, field, index);
		}
		if (field->is_optional()){
			return this->EncodeOptional(message, field, index);
		}
		if (field->is_repeated()){
			return this->EncodeRepeted(message, field, index);
		}
		return false;
	}

	bool MessageEncoder::EncodeTable(pb::Message& message, const pb::FieldDescriptor* field, int index)
	{
		if (lua_isnil(this->mLua, index)) {
			return true;
		}

		if (!lua_istable(this->mLua, index)) {
			luaL_error(this->mLua, "encode_table field isn't a table, field=%s", field->full_name().c_str());
			return false;
		}

		const pb::Reflection* reflection = message.GetReflection();
		const pb::Descriptor* descriptor = field->message_type();
		if(descriptor->field_count() != 2)
		{
			return false;
		}
		const pb::FieldDescriptor* key = descriptor->field(0);
		const pb::FieldDescriptor* value = descriptor->field(1);

		lua_pushnil(this->mLua);
		while (lua_next(this->mLua, index))
		{
			pb::Message* submessage = reflection->AddMessage(&message, field);
			if(!this->EncodeField(*submessage, key, lua_absindex(this->mLua, -2)))
			{
				return false;
			}
			if(!this->EncodeField(*submessage, value, lua_absindex(this->mLua, -1)))
			{
				return false;
			}
			lua_pop(this->mLua, 1);
		}
		return true;
	}

	bool MessageEncoder::EncodeSingle(pb::Message& message, const pb::FieldDescriptor* field, int index)
	{
		const pb::Reflection* reflection = message.GetReflection();
		switch (field->cpp_type())
		{
			case pb::FieldDescriptor::CPPTYPE_DOUBLE:
				reflection->SetDouble(&message, field, (double)lua_tonumber(this->mLua, index));
				break;
			case pb::FieldDescriptor::CPPTYPE_FLOAT:
				reflection->SetFloat(&message, field, (float)lua_tonumber(this->mLua, index));
				break;
			case pb::FieldDescriptor::CPPTYPE_INT32:
				reflection->SetInt32(&message, field, (pb::int32)lua_tointeger(this->mLua, index));
				break;
			case pb::FieldDescriptor::CPPTYPE_UINT32:
				reflection->SetUInt32(&message, field, (pb::uint32)lua_tointeger(this->mLua, index));
				break;
			case pb::FieldDescriptor::CPPTYPE_INT64:
				reflection->SetInt64(&message, field, (pb::int64)lua_tointeger(this->mLua, index));
				break;
			case pb::FieldDescriptor::CPPTYPE_UINT64:
				reflection->SetUInt64(&message, field, (pb::uint64)lua_tointeger(this->mLua, index));
				break;
			case pb::FieldDescriptor::CPPTYPE_ENUM:
				reflection->SetEnumValue(&message, field, (int)lua_tointeger(this->mLua, index));
				break;
			case pb::FieldDescriptor::CPPTYPE_BOOL:
				reflection->SetBool(&message, field, lua_toboolean(this->mLua, index) != 0);
				break;
			case pb::FieldDescriptor::CPPTYPE_STRING:
			{
				size_t length = 0;
				const char* bytes = lua_tolstring(this->mLua, index, &length);
				reflection->SetString(&message, field, std::string(bytes, length));
			}
				break;
			case pb::FieldDescriptor::CPPTYPE_MESSAGE:
			{
				pb::Message* submessage = reflection->MutableMessage(&message, field);
				if (submessage->GetTypeName() == "google.protobuf.Any")
				{
					pb::Message * message = Lua::PtrProxy<pb::Message>::Read(this->mLua, index);
					if (message == nullptr)
					{
						return false;
					}
					std::unique_ptr<pb::Message> pb(message);
					dynamic_cast<pb::Any*>(submessage)->PackFrom(*message);
					return true;
				}

				if (!this->EncoddeMessage(*submessage, field->message_type(), index))
				{
					return false;
				}
			}
				break;
			default:
				luaL_error(this->mLua, "encode_single field unknow type, field=%s", field->full_name().c_str());
				return false;
		}
		return true;
	}

	bool MessageEncoder::EncodeMutiple(pb::Message& message, const pb::FieldDescriptor* field, int index)
	{
		const pb::Reflection* reflection = message.GetReflection();
		switch (field->cpp_type())
		{
		case pb::FieldDescriptor::CPPTYPE_DOUBLE:
			reflection->AddDouble(&message, field, (double)lua_tonumber(this->mLua, index));
			break;
		case pb::FieldDescriptor::CPPTYPE_FLOAT:
			reflection->AddFloat(&message, field, (float)lua_tonumber(this->mLua, index));
			break;
		case pb::FieldDescriptor::CPPTYPE_INT32:
			reflection->AddInt32(&message, field, (pb::int32)lua_tointeger(this->mLua, index));
			break;
		case pb::FieldDescriptor::CPPTYPE_UINT32:
			reflection->AddUInt32(&message, field, (pb::uint32)lua_tointeger(this->mLua, index));
			break;
		case pb::FieldDescriptor::CPPTYPE_INT64:
			reflection->AddInt64(&message, field, (pb::int64)lua_tointeger(this->mLua, index));
			break;
		case pb::FieldDescriptor::CPPTYPE_UINT64:
			reflection->AddUInt64(&message, field, (pb::uint64)lua_tointeger(this->mLua, index));
			break;
		case pb::FieldDescriptor::CPPTYPE_ENUM:
			reflection->AddEnumValue(&message, field, (int)lua_tointeger(this->mLua, index));
			break;
		case pb::FieldDescriptor::CPPTYPE_BOOL:
			reflection->AddBool(&message, field, lua_toboolean(this->mLua, index) != 0);
			break;
		case pb::FieldDescriptor::CPPTYPE_STRING:
		{
			size_t length = 0;
			const char* bytes = lua_tolstring(this->mLua, index, &length);
			reflection->AddString(&message, field, std::string(bytes, length));
		}
			break;
		case pb::FieldDescriptor::CPPTYPE_MESSAGE:
		{
			pb::Message* submessage = reflection->AddMessage(&message, field);
            if(submessage->GetTypeName() == "google.protobuf.Any")
            {
                pb::Message * message = Lua::PtrProxy<pb::Message>::Read(this->mLua, index);
                if(message == nullptr)
                {
                    return false;
                }
				std::unique_ptr<pb::Message> pb(message);
                dynamic_cast<pb::Any*>(submessage)->PackFrom(*message);
                return true;
            }
			if(!this->EncoddeMessage(*submessage, field->message_type(), index))
			{
				return false;
			}
		}
			break;
		default:
			luaL_error(this->mLua, "encode_multiple field unknown type, field=%s", field->full_name().c_str());
			return false;
		}
		return true;
	}

	bool MessageEncoder::EncodeRepeted(pb::Message& message, const pb::FieldDescriptor* field, int index)
	{
		if (lua_isnil(this->mLua, index)) {
			return true;
		}

		if (!lua_istable(this->mLua, index)) {
			luaL_error(this->mLua, "encode_repeated field isn't a table, field=%s", field->full_name().c_str());
			return false;
		}

		int count = (int)luaL_len(this->mLua, index);
		for (int i = 0; i < count; i++)
		{
			lua_geti(this->mLua, index, i + 1);
			if(!this->EncodeMutiple(message, field, lua_absindex(this->mLua, -1)))
			{
				return false;
			}
			lua_pop(this->mLua, 1);
		}
		return true;
	}

	bool MessageEncoder::EncodeOptional(pb::Message& message, const pb::FieldDescriptor* field, int index)
	{
		if (lua_isnil(this->mLua, index)) {
			return true;
		}

		return this->EncodeSingle(message, field, index);
	}

	bool MessageEncoder::EncodeRequired(pb::Message& message, const pb::FieldDescriptor* field, int index)
	{
		if (lua_isnil(this->mLua, index)) {
			luaL_error(this->mLua, "encode_required field nil, field=%s", field->full_name().c_str());
			return true;
		}

		return this->EncodeSingle(message, field, index);
	}
}