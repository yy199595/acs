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
namespace Tendo
{
	class ServiceMethod;
	class InnerNetTcpClient;
	class RpcService : public Component, public IServiceBase
	{
	public:
		const std::string& GetServer() const { return this->mCluster; }
	protected:
		bool LateAwake() final;
	public:
		virtual int Invoke(const std::string& method, std::shared_ptr<Msg::Packet> message) = 0;
	private:
		std::string mCluster;
	};
}