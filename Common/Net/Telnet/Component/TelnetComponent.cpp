//
// Created by 64658 on 2025/3/4.
//

#include "TelnetComponent.h"
#include "Entity/Actor/App.h"
#include "Server/Config/CodeConfig.h"
#include "Router/Component/RouterComponent.h"
namespace acs
{
	TelnetComponent::TelnetComponent()
	{
		this->mRouter = nullptr;
	}

	bool TelnetComponent::LateAwake()
	{
		this->mRouter = this->GetComponent<RouterComponent>();
		return true;
	}

	bool TelnetComponent::OnListen(tcp::Socket* socket) noexcept
	{
		int id = this->mNumberPool.BuildNumber();
		Asio::Context & main = this->mApp->GetContext();
		std::shared_ptr<telnet::Client> client = std::make_shared<telnet::Client>(id, socket, main, this);
		{
			const std::string msg("===== welcome connect acs server =====");
			std::unique_ptr<telnet::Response> welcome = std::make_unique<telnet::Response>(msg);
			{
				client->Send(std::move(welcome));
				this->mClients.emplace(id, client);
			}
		}
		return true;
	}

	void TelnetComponent::OnClientError(int id, int code)
	{
		auto iter = this->mClients.find(id);
		if(iter != this->mClients.end())
		{
			this->mClients.erase(iter);
		}
	}

	bool TelnetComponent::Send(int id, const std::string& msg)
	{
		auto iter = this->mClients.find(id);
		if(iter == this->mClients.end())
		{
			return false;
		}
		iter->second->Send(std::make_unique<telnet::Response>(msg));
		return true;
	}

	bool TelnetComponent::Send(int id, std::unique_ptr<telnet::Response> response)
	{
		auto iter = this->mClients.find(id);
		if(iter == this->mClients.end())
		{
			return false;
		}
		iter->second->Send(std::move(response));
		return true;
	}

	void TelnetComponent::OnMessage(int id, telnet::Request* request, telnet::Response* ) noexcept
	{
		const std::string & cmd = request->GetCmd();
		if(cmd == "quit")
		{
			auto iter = this->mClients.find(id);
			if(iter != this->mClients.end())
			{
				iter->second->StartClose();
			}
			return;
		}
		this->mApp->StartCoroutine([request, this, id, cmd]()
		{
			std::string response("[error] unknown cmd");

			json::w::Document document;
			{
				document.Add("cmd", cmd);
				if(!request->GetArgs().empty())
				{
					std::unique_ptr<json::w::Value> jsonArray = document.AddArray("args");
					for(const std::string & args : request->GetArgs())
					{
						jsonArray->Push(args);
					}
				}
			}
			int code = XCode::Ok;
			std::string message;
			do
			{
				std::unique_ptr<rpc::Message> rpcMessage = this->mApp->Make("TelnetSystem.Run");
				if(rpcMessage == nullptr)
				{
					code = XCode::MakeTcpRequestFailure;
					break;
				}
				document.Encode(rpcMessage->Body());
				int nodeId = this->mApp->GetNodeId();
				std::unique_ptr<rpc::Message> rpcResponse = this->mRouter->Call(nodeId, std::move(rpcMessage));
				if(rpcResponse == nullptr)
				{
					code = XCode::Failure;
					message = "rpc response fail";
					break;
				}
				code = rpcResponse->GetCode();
				if(code == XCode::Ok)
				{
					message = rpcResponse->GetBody();
					break;
				}
			}
			while(false);
			std::unique_ptr<telnet::Response> telnetResponse = std::make_unique<telnet::Response>();
			{
				std::string name = CodeConfig::Inst()->GetDesc(code);
				telnetResponse->Append(fmt::format("=============== [{}] ===============", name));
				if(!message.empty())
				{
					telnetResponse->Append(fmt::format("\r\n{}", message));
					telnetResponse->Append(fmt::format("\r\n=============== [{}ms] ===============", request->GetCostTime()));
				}
			}
			this->Send(id, std::move(telnetResponse));
			delete request;
		});
	}
}