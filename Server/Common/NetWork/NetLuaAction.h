#pragma once
#include<XCode/XCode.h>
#include<Define/CommonDef.h>
#include<Protocol/Common.pb.h>
#include<Script/LuaParameter.h>
#include<NetWork/TcpClientSession.h>
using namespace PB;
namespace SoEasy
{
	class NetLuaAction
	{
	public:
		NetLuaAction(lua_State * lua, const std::string name, int action_ref);
		virtual ~NetLuaAction();
	public:
		const std::string & GetActionName() { return mActionName; }
		XCode Invoke(const shared_ptr<NetWorkPacket> requestData); //本地调用
		XCode Invoke(const std::string & address, const shared_ptr<NetWorkPacket> requestData);
	private:
		int mLocalref;
		int mProxyref;
		int mActionref;
		lua_State * luaEnv;
		std::string mActionName;
	};
}
