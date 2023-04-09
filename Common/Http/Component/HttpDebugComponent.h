#pragma once
#include"Rpc/Client/Message.h"
#include"HttpListenComponent.h"

namespace Tendo
{
	class HttpDebugComponent final : public HttpListenComponent
	{
	public:
		HttpDebugComponent() = default;
	private:
		bool LateAwake() final;
		bool OnDelClient(const std::string& address) final;
		void OnRequest(std::shared_ptr<Http::Request> request) final;
	private:
		void Call(const std::string & target, std::shared_ptr<Rpc::Packet>& message);
		bool GetAddress(const std::string& service, long long id, std::string& address);
	private:
		class NodeMgrComponent* mNodeComponent;
		class InnerNetComponent* mInnerComponent;
	};
}