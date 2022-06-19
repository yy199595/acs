#pragma once
#include "Component/Component.h"
#include "Network/Telnet/TelnetClientContext.h"

namespace Sentry
{
	class TelnetProto : public Tcp::ProtoMessage
	{
	 public:
		TelnetProto() = default;
		~TelnetProto() = default;
	 public:
		void Add(const std::string & conetnt);
		void Add(const char * str, size_t size);
		int Serailize(std::ostream & os) final; //返回剩余要发送的字节数
	 private:
		std::list<std::string> mContents;
	};
}

namespace Tcp
{
	class TelnetClientContext;
}

namespace Sentry
{
	using ConsoleFunction = std::function<bool(const std::string&, std::vector<std::string>&)>;
	class ConsoleComponent : public Component, public ISocketListen, public IStart
	{
	 public:
		ConsoleComponent() = default;
		~ConsoleComponent() final = default;
	 public:
		bool OnStart() final;
		bool LateAwake() final;
		void OnListen(std::shared_ptr<SocketProxy> socket) final;
		void OnClientError(std::shared_ptr<Tcp::TelnetClientContext> clientContext);
		void OnReceive(std::shared_ptr<Tcp::TelnetClientContext> clientContext, const std::string & message);
	 private:
		bool Offset(const std::string& parameter, std::vector<string>& response);
		bool Help(const std::string& parameter, std::vector<std::string>& response);
		bool Start(const std::string& parameter, std::vector<std::string>& response);
		bool Close(const std::string& parameter, std::vector<std::string>& response);
		bool Hotfix(const std::string& parameter, std::vector<std::string>& response);
		bool Services(const std::string& parameter, std::vector<std::string>& response);
	 private:
		class TaskComponent* mTaskComponent;
		std::unordered_map<std::string, ConsoleFunction> mFunctionMap;
		std::set<std::shared_ptr<Tcp::TelnetClientContext>> mTelnetClients;
	};
}

