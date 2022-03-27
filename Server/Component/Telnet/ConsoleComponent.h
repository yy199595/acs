#pragma once
#include "Component/Component.h"
#include "Network/Telnet/TelnetClient.h"
namespace Sentry
{
	class TelnetClient;
	using ConsoleFunction = std::function<bool(const std::string&, std::vector<std::string>&)>;
	class ConsoleComponent : public Component, public ISocketListen
	{
	 public:
		ConsoleComponent() = default;
		~ConsoleComponent() final = default;

	 public:
		bool Awake() final;
		bool LateAwake() final;
		void OnListen(std::shared_ptr<SocketProxy> socket) final;
	 private:

		bool Offset(const std::string& parameter, std::vector<string>& response);
		bool Help(const std::string& parameter, std::vector<std::string>& response);
		bool Start(const std::string& parameter, std::vector<std::string>& response);
		bool Close(const std::string& parameter, std::vector<std::string>& response);
		bool Hotfix(const std::string& parameter, std::vector<std::string>& response);
		bool Services(const std::string& parameter, std::vector<std::string>& response);
	 private:
		void HandleConsoleClient(std::shared_ptr<TelnetClient> telnetClient);
	 private:
		class TaskComponent* mTaskComponent;
		std::unordered_map<std::string, ConsoleFunction> mFunctionMap;
	};
}

