#include"BindLuaFunction.h"
#include<Define/CommonDef.h>
#include<Define/ClassStatement.h>
#include<google/protobuf/message.h>
#include<google/protobuf/descriptor.h>
#include<google/protobuf/util/json_util.h>

namespace SoEasy
{
	class GameObject;
	using namespace google::protobuf;
	bool BindLuaFunction::Invoke(GameObject * gameObject, const std::string & jsonString)
	{
		const char * data = jsonString.c_str();
		const size_t size = jsonString.size();
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		SayNoAssertRetFalse_F(lua_isfunction(this->mLua, -1));
		PtrProxy<GameObject>::Write(this->mLua, gameObject);
		lua_getglobal(this->mLua, TypeReflection<GameObject>::Name);
		lua_setmetatable(this->mLua, -2);
		SayNoAssertRetFalse_F(lua_getfunction(this->mLua, "JsonUtil", "Decode"));
		lua_pushlstring(this->mLua, data, size);
		if (lua_pcall(this->mLua, 1, 1, 0) == 0)
		{
			lua_remove(this->mLua, -2);
			if (lua_pcall(this->mLua, 2, 0, 0) != 0)
			{
				SayNoDebugError(lua_tostring(this->mLua, -1));
				return false;
			}
			return true;
		}
		SayNoDebugError(lua_tostring(this->mLua, -1));
		return false;
	}
}
	

namespace SoEasy
{ 
	bool BindClientLuaFunction::Invoke(GameObject * gameObject, const std::string & data)
	{
		const Descriptor * desc = DescriptorPool::generated_pool()->FindMessageTypeByName(this->mPbName);
		SayNoAssertRetVal(desc, this->mPbName, false);
		Message * message = MessageFactory::generated_factory()->GetPrototype(desc)->New();
		SayNoAssertRetFalse_F(message->ParseFromString(data));
		std::string jsonData = "";
		SayNoAssertRetFalse_F(util::MessageToJsonString(*message, &jsonData).ok());

		return BindLuaFunction::Invoke(gameObject, jsonData);
	}

}

