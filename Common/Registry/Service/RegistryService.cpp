//
// Created by 64658 on 2025/4/9.
//

#include "RegistryService.h"
#include "com/com.pb.h"
#include "Util/Tools/Math.h"
#include "Entity/Actor/App.h"
#include "Util/Tools/TimeHelper.h"
#include "Router/Component/RouterComponent.h"
#include "Sqlite/Component/SqliteComponent.h"
namespace acs
{
	RegistryService::RegistryService()
	{
		this->mSqlite = nullptr;
		this->mRouter = nullptr;
		REGISTER_JSON_CLASS_MUST_FIELD(node::Info, id);
		REGISTER_JSON_CLASS_FIELD(node::Info, last_time);
		REGISTER_JSON_CLASS_MUST_FIELD(node::Info, name);
		REGISTER_JSON_CLASS_MUST_FIELD(node::Info, listen);
	}

	bool RegistryService::OnInit()
	{
		BIND_RPC_METHOD(RegistryService::Add)
		BIND_RPC_METHOD(RegistryService::Del)
		BIND_RPC_METHOD(RegistryService::Ping)
		LOG_CHECK_RET_FALSE(this->mSqlite = this->GetComponent<SqliteComponent>())
		LOG_CHECK_RET_FALSE(this->mRouter = this->GetComponent<RouterComponent>())
		this->mSqlite->Build("register_delete", fmt::format("DELETE FROM {} WHERE id=?", node::REGISTRY_LIST));
		this->mSqlite->Build("register_update", fmt::format("UPDATE {} SET last_time=? WHERE id=?", node::REGISTRY_LIST));
		this->mSqlite->Build("register_replace", fmt::format("REPLACE INTO {}(id,name,listen,last_time)VALUES(?,?,?,?)", node::REGISTRY_LIST));
		return true;
	}

	void RegistryService::OnStart()
	{
		std::string sql = fmt::format("SELECT * FROM {}", node::REGISTRY_LIST);
		std::unique_ptr<sqlite::Response> response = this->mSqlite->Run(sql);

		for (const std::unique_ptr<json::r::Document>& document: response->result)
		{
			node::Info nodeInfo;
			if (nodeInfo.Decode(*document))
			{
				this->mNodeList.emplace(nodeInfo.id, nodeInfo);
			}
		}
	}

	void RegistryService::OnSecondUpdate(int tick) noexcept
	{
		if(tick % node::TIME_OUT == 0)
		{
			long long nowTime = help::Time::NowSec();
			for(auto iter = this->mNodeList.begin(); iter != this->mNodeList.end(); iter++)
			{
				const node::Info& info = iter->second;
				if(nowTime - info.last_time >= node::TIME_OUT * 2) //长时间没有动静了
				{

				}
			}
		}
		this->NoticeMessage();
	}

	int RegistryService::Add(const rpc::Message& request)
	{
		json::r::Document document;
		const std::string & message = request.GetBody();
		if(!document.Decode(message, YYJSON_READ_INSITU))
		{
			return XCode::CallArgsError;
		}
		node::Info nodeInfo;
		json::r::Value listenObject;
		nodeInfo.sockId = request.SockId();
		nodeInfo.last_time = help::Time::NowSec();
		LOG_ERROR_CHECK_ARGS(document.Get("id", nodeInfo.id))
		LOG_ERROR_CHECK_ARGS(document.Get("name", nodeInfo.name))
		LOG_ERROR_CHECK_ARGS(document.Get("listen", listenObject))

		nodeInfo.listen = listenObject.ToString();
		if(!this->mSqlite->Invoke("register_replace", nodeInfo.id, nodeInfo.name, nodeInfo.listen, nodeInfo.last_time))
		{
			return XCode::Failure;
		}
		this->mNodeList[nodeInfo.id] = nodeInfo;
		{
			this->Broadcast(nodeInfo);
			LOG_INFO("[register ok] {}", request.GetBody());
		}
		return XCode::Ok;
	}

	int RegistryService::Del(const rpc::Message& request)
	{
		int id = 0;
		if(!help::Math::ToNumber(request.GetBody(), id))
		{
			return XCode::CallArgsError;
		}
		auto iter = this->mNodeList.find(id);
		if(iter == this->mNodeList.end())
		{
			return XCode::Failure;
		}
		if(!this->mSqlite->Invoke("register_delete", id))
		{
			return XCode::Failure;
		}
		this->mNodeList.erase(iter);
		{
			this->Broadcast(id);
		}
		return true;
	}

	int RegistryService::Find(const rpc::Message & request, rpc::Message& response)
	{
		json::w::Document document;
		const std::string & node = request.GetBody();
		std::unique_ptr<json::w::Value> jsonArray = document.AddArray("list");
		for(auto iter = this->mNodeList.begin(); iter != this->mNodeList.end(); iter++)
		{
			const node::Info & info = iter->second;
			if(info.sockId > 0 && (node.empty() || info.name == node))
			{
				std::unique_ptr<json::w::Value> jsonObject = jsonArray->AddObject();
				{
					jsonObject->Add("id", info.id);
					jsonObject->Add("name", info.name);
					jsonObject->AddObject("listen", info.listen);
				}
			}
		}
		response.SetProto(rpc::proto::json);
		response.SetContent(document.JsonString());
		return XCode::Ok;
	}

	int RegistryService::Ping(const rpc::Message& request)
	{
		int actorId = 0;
		if(!help::Math::ToNumber(request.GetBody(), actorId))
		{
			return XCode::CallArgsError;
		}
		auto iter = this->mNodeList.find(actorId);
		if(iter == this->mNodeList.end())
		{
			return XCode::Failure;
		}
		iter->second.sockId = request.SockId();
		iter->second.last_time = help::Time::NowSec();
		if(!this->mSqlite->Invoke("register_update", iter->second.last_time, actorId))
		{
			return XCode::Ok;
		}
		this->NoticeMessage(iter->second);
		return XCode::Ok;
	}

	void RegistryService::NoticeMessage()
	{
		for (auto iter = this->mNodeList.begin(); iter != this->mNodeList.end(); iter++)
		{
			int sockId = iter->second.sockId;
			if((sockId > 0) && (iter->second.addQueue.empty() || iter->second.delQueue.empty()))
			{
				this->NoticeMessage(iter->second);
			}
		}
	}

	void RegistryService::NoticeMessage(node::Info& nodeInfo)
	{
		int sockId = nodeInfo.sockId;
		if (sockId > 0 && !nodeInfo.addQueue.empty())
		{
			std::unordered_set<int> doneMessages;
			for (int id: nodeInfo.addQueue)
			{
				node::Info* nodeInfo = this->Find(id);
				if (nodeInfo != nullptr)
				{
					json::w::Document document;
					nodeInfo->Encode(document);
					std::unique_ptr<rpc::Message> request = this->mApp->Make("NodeSystem.Add");
					if (request != nullptr)
					{
						request->SetSockId(sockId);
						request->SetContent(document);
						if (this->mRouter->Send(sockId, request) == XCode::Ok)
						{
							doneMessages.emplace(id);
						}
					}
				}
			}

			nodeInfo.addQueue.erase(std::remove_if(nodeInfo.addQueue.begin(),
					nodeInfo.addQueue.end(), [&doneMessages](int a)
					{
						return doneMessages.find(a) != doneMessages.end();
					}), nodeInfo.addQueue.end());

			doneMessages.clear();
		}
		if (sockId > 0 && !nodeInfo.delQueue.empty())
		{
			std::unordered_set<int> doneMessages;
			for (int id: nodeInfo.delQueue)
			{
				std::unique_ptr<rpc::Message> request = this->mApp->Make("NodeSystem.Del");
				if (request != nullptr)
				{
					request->SetSockId(sockId);
					request->SetContent(std::to_string(id));
					if (this->mRouter->Send(sockId, request) == XCode::Ok)
					{
						doneMessages.emplace(id);
					}
				}
			}

			nodeInfo.delQueue.erase(std::remove_if(nodeInfo.delQueue.begin(),
					nodeInfo.delQueue.end(), [&doneMessages](int a)
					{
						return doneMessages.find(a) != doneMessages.end();
					}), nodeInfo.delQueue.end());
		}
	}

	node::Info* RegistryService::Find(int id)
	{
		auto iter = this->mNodeList.find(id);
		return iter == this->mNodeList.end() ? nullptr : &iter->second;
	}

	void RegistryService::Broadcast(int id)
	{
		if(!this->mApp->Equal(id))
		{
			for (auto iter = this->mNodeList.begin(); iter != this->mNodeList.end(); iter++)
			{
				iter->second.delQueue.emplace_back(id);
			}
			this->NoticeMessage();
		}
	}

	void RegistryService::Broadcast(node::Info & nodeInfo)
	{
		if(!this->mApp->Equal(nodeInfo.id))
		{
			for(auto iter = this->mNodeList.begin(); iter != this->mNodeList.end(); iter++)
			{
				nodeInfo.addQueue.emplace_back(iter->second.id);
				iter->second.addQueue.emplace_back(nodeInfo.id);
			}
			this->NoticeMessage();
		}
	}
}