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
	class ConsoleComponent : public Component, public ISocketListen
	{
	 public:
		ConsoleComponent() = default;
		~ConsoleComponent() final = default;
	 public:
		bool LateAwake() final;
		void OnClientError(const std::string & address);
		bool OnListen(std::shared_ptr<SocketProxy> socket) final;
		void OnReceive(const std::string & address, const std::string & message);

    private:
        bool Invoke(std::vector<std::string> & request, std::shared_ptr<TelnetProto> response);
	 private:
		bool Offset(const std::string& parameter, std::vector<string>& response);
		bool Help(const std::string& parameter, std::vector<std::string>& response);
		bool Start(const std::string& parameter, std::vector<std::string>& response);
		bool Close(const std::string& parameter, std::vector<std::string>& response);
		bool Hotfix(const std::string& parameter, std::vector<std::string>& response);
		bool Services(const std::string& parameter, std::vector<std::string>& response);

	private:
		std::shared_ptr<Tcp::TelnetClientContext> GetClient(const std::string & address);
	 private:
		class TaskComponent* mTaskComponent;
		std::unordered_map<std::string, ConsoleFunction> mFunctionMap;
		std::unordered_map<std::string, std::shared_ptr<Tcp::TelnetClientContext>> mTelnetClients;
	};
}

