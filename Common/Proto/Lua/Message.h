//
// Created by mac on 2022/6/1.
//

#ifndef SERVER_MESSAGE_H
#define SERVER_MESSAGE_H
#include"Lua/Engine/Define.h"
#include"Proto/Component/ProtoComponent.h"
namespace acs
{
	class MessageDecoder //转lua
	{
	 public:
		MessageDecoder(lua_State * lua, ProtoComponent * component);
		bool Decode(const pb::Message & message);
	 private:
		bool DecodeField(const pb::Message & message, const pb::FieldDescriptor * field);
		bool DecodeSingle(const pb::Message & message, const pb::FieldDescriptor * field);
		bool DecodeTable(const pb::Message & message, const pb::FieldDescriptor * field);
		bool DecodeRepeted(const pb::Message & message, const pb::FieldDescriptor * field);
		bool DecodeRequired(const pb::Message & message, const pb::FieldDescriptor * field);
		bool DecodeOptional(const pb::Message & message, const pb::FieldDescriptor * field);
		bool DecodeMutiple(const pb::Message & message, const pb::FieldDescriptor * field, int index);
	 private:
		lua_State * mLua;
		ProtoComponent * mMsgComponent;
	};
}

namespace acs
{
	class MessageEncoder
	{
	 public:
		explicit MessageEncoder(lua_State * lua);
	 public:
		bool Encode(pb::Message & proto, int index);
	 private:
		bool EncodeField(pb::Message & message, const pb::FieldDescriptor * field, int index);
		bool EncodeSingle(pb::Message & message, const pb::FieldDescriptor * field, int index);
		bool EncodeTable(pb::Message & message, const pb::FieldDescriptor * field, int index);
		bool EncodeRepeted(pb::Message & message, const pb::FieldDescriptor * field, int index);
		bool EncodeRequired(pb::Message & message, const pb::FieldDescriptor * field, int index);
		bool EncodeOptional(pb::Message & message, const pb::FieldDescriptor * field, int index);
		bool EncodeMutiple(pb::Message & message, const pb::FieldDescriptor * field, int index);
		bool EncoddeMessage(pb::Message & message, const pb::Descriptor * descriptor, int index);
	 private:
		lua_State * mLua;
	};
}

namespace lua
{
	namespace MessageEx
	{
		extern int New(lua_State * lua);
		extern int Import(lua_State* lua);
		extern int Encode(lua_State * lua);
		extern int Decode(lua_State * lua);
		extern int ToJson(lua_State * lua);
	};
}


#endif //SERVER_MESSAGE_H
