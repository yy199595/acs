#pragma once

#include<memory>
#include<vector>
#include"Entity/Component/Component.h"
#include<google/protobuf/message.h>
using namespace google::protobuf;
namespace Msg
{
    class Packet;
};
namespace Lua
{
	class LuaModule;
}
namespace Tendo
{
	class ServiceMethod;
	class InnerNetTcpClient;
	class RpcService : public Component, public IServiceBase
	{
	protected:
		bool LateAwake() final;
		virtual bool OnInit() = 0; //注册rpc回调
		virtual void OnStop() = 0; //服务关闭调用(协程)
		virtual void OnStart() = 0; //服务启动调用(协程)
		virtual void OnComplete() { } //服务器启动成功调用(协程)
	public:
		Lua::LuaModule * GetLuaModule() { return this->mLuaModule; }
		const std::string& GetServer() const { return this->mCluster; }
		virtual int Invoke(const std::string& method, std::shared_ptr<Msg::Packet> message) = 0;
	private:
		std::string mCluster;
		Lua::LuaModule * mLuaModule;
	};
}