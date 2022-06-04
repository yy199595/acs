//
// Created by yjz on 2022/6/3.
//
#include"Message.h"

namespace Sentry
{
	MessageEncoder::MessageEncoder(lua_State* lua, MessageComponent* component)
	{
		this->mLua = lua;
		this->mMsgComponent = component;
	}

	std::shared_ptr<Message> MessageEncoder::Encode(const std::string& proto, int index)
	{
		if(!lua_istable(this->mLua, index))
		{
			return nullptr;
		}
		std::shared_ptr<Message> message = this->mMsgComponent->New(proto);
		if(message == nullptr)
		{
			return nullptr;
		}
		index = lua_absindex(this->mLua, index);
		const Descriptor* descriptor = message->GetDescriptor();
		if(!this->EncoddeMessage(*message, descriptor, index))
		{
			return nullptr;
		}
		return message;
	}

	bool MessageEncoder::EncoddeMessage(Message& message, const Descriptor* descriptor, int index)
	{
		if (!lua_istable(this->mLua, index)) {
			luaL_error(this->mLua, "encode_message field isn't a table, field=%s", descriptor->full_name().c_str());
			return false;
		}
		for (int i = 0; i < descriptor->field_count(); i++)
		{
			const FieldDescriptor* field = descriptor->field(i);
			lua_getfield(this->mLua, index, field->name().c_str());
			if(!this->EncodeField(message, field, lua_absindex(this->mLua, -1)))
			{
				return false;
			}
			lua_pop(this->mLua, 1);
		}
		return true;
	}

	bool MessageEncoder::EncodeField(Message& message, const FieldDescriptor* field, int index)
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

	bool MessageEncoder::EncodeTable(Message& message, const FieldDescriptor* field, int index)
	{
		if (lua_isnil(this->mLua, index)) {
			return true;
		}

		if (!lua_istable(this->mLua, index)) {
			luaL_error(this->mLua, "encode_table field isn't a table, field=%s", field->full_name().c_str());
			return false;
		}

		const Reflection* reflection = message.GetReflection();
		const Descriptor* descriptor = field->message_type();
		if(descriptor->field_count() != 2)
		{
			return false;
		}
		const FieldDescriptor* key = descriptor->field(0);
		const FieldDescriptor* value = descriptor->field(1);

		lua_pushnil(this->mLua);
		while (lua_next(this->mLua, index))
		{
			Message* submessage = reflection->AddMessage(&message, field);
			if(!this->EncodeField(*submessage, key, lua_absindex(this->mLua, -2)))
			{
				return false;
			}
			if(this->EncodeField(*submessage, value, lua_absindex(this->mLua, -1)))
			{
				return false;
			}
			lua_pop(this->mLua, 1);
		}
		return true;
	}

	bool MessageEncoder::EncodeSingle(Message& message, const FieldDescriptor* field, int index)
	{
		const Reflection* reflection = message.GetReflection();
		switch (field->cpp_type())
		{
		case FieldDescriptor::CPPTYPE_DOUBLE:
			reflection->SetDouble(&message, field, (double)lua_tonumber(this->mLua, index));
			break;
		case FieldDescriptor::CPPTYPE_FLOAT:
			reflection->SetFloat(&message, field, (float)lua_tonumber(this->mLua, index));
			break;
		case FieldDescriptor::CPPTYPE_INT32:
			reflection->SetInt32(&message, field, (int32)lua_tointeger(this->mLua, index));
			break;
		case FieldDescriptor::CPPTYPE_UINT32:
			reflection->SetUInt32(&message, field, (uint32)lua_tointeger(this->mLua, index));
			break;
		case FieldDescriptor::CPPTYPE_INT64:
			reflection->SetInt64(&message, field, (int64)lua_tointeger(this->mLua, index));
			break;
		case FieldDescriptor::CPPTYPE_UINT64:
			reflection->SetUInt64(&message, field, (uint64)lua_tointeger(this->mLua, index));
			break;
		case FieldDescriptor::CPPTYPE_ENUM:
			reflection->SetEnumValue(&message, field, (int)lua_tointeger(this->mLua, index));
			break;
		case FieldDescriptor::CPPTYPE_BOOL:
			reflection->SetBool(&message, field, lua_toboolean(this->mLua, index) != 0);
			break;
		case FieldDescriptor::CPPTYPE_STRING:
		{
			size_t length = 0;
			const char* bytes = lua_tolstring(this->mLua, index, &length);
			reflection->SetString(&message, field, string(bytes, length));
		}
			break;
		case FieldDescriptor::CPPTYPE_MESSAGE:
		{
			Message* submessage = reflection->MutableMessage(&message, field);
			if(!this->EncoddeMessage(*submessage, field->message_type(), index))
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

	bool MessageEncoder::EncodeMutiple(Message& message, const FieldDescriptor* field, int index)
	{
		const Reflection* reflection = message.GetReflection();
		switch (field->cpp_type())
		{
		case FieldDescriptor::CPPTYPE_DOUBLE:
			reflection->AddDouble(&message, field, (double)lua_tonumber(this->mLua, index));
			break;
		case FieldDescriptor::CPPTYPE_FLOAT:
			reflection->AddFloat(&message, field, (float)lua_tonumber(this->mLua, index));
			break;
		case FieldDescriptor::CPPTYPE_INT32:
			reflection->AddInt32(&message, field, (int32)lua_tointeger(this->mLua, index));
			break;
		case FieldDescriptor::CPPTYPE_UINT32:
			reflection->AddUInt32(&message, field, (uint32)lua_tointeger(this->mLua, index));
			break;
		case FieldDescriptor::CPPTYPE_INT64:
			reflection->AddInt64(&message, field, (int64)lua_tointeger(this->mLua, index));
			break;
		case FieldDescriptor::CPPTYPE_UINT64:
			reflection->AddUInt64(&message, field, (uint64)lua_tointeger(this->mLua, index));
			break;
		case FieldDescriptor::CPPTYPE_ENUM:
			reflection->AddEnumValue(&message, field, (int)lua_tointeger(this->mLua, index));
			break;
		case FieldDescriptor::CPPTYPE_BOOL:
			reflection->AddBool(&message, field, lua_toboolean(this->mLua, index) != 0);
			break;
		case FieldDescriptor::CPPTYPE_STRING:
		{
			size_t length = 0;
			const char* bytes = lua_tolstring(this->mLua, index, &length);
			reflection->AddString(&message, field, std::string(bytes, length));
		}
			break;
		case FieldDescriptor::CPPTYPE_MESSAGE:
		{
			Message* submessage = reflection->AddMessage(&message, field);
			if(!this->EncoddeMessage(*submessage, field->message_type(), index))
			{
				return false;
			}
		}
			break;
		default:
			luaL_error(this->mLua, "encode_multiple field unknow type, field=%s", field->full_name().c_str());
			return false;
		}
		return true;
	}

	bool MessageEncoder::EncodeRepeted(Message& message, const FieldDescriptor* field, int index)
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

	bool MessageEncoder::EncodeOptional(Message& message, const FieldDescriptor* field, int index)
	{
		if (lua_isnil(this->mLua, index)) {
			return true;
		}

		return this->EncodeSingle(message, field, index);
	}

	bool MessageEncoder::EncodeRequired(Message& message, const FieldDescriptor* field, int index)
	{
		if (lua_isnil(this->mLua, index)) {
			luaL_error(this->mLua, "encode_required field nil, field=%s", field->full_name().c_str());
			return true;
		}

		return this->EncodeSingle(message, field, index);
	}
}