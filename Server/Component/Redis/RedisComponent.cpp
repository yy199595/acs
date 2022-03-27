#include"RedisComponent.h"

#include"App/App.h"
#include"Util/FileHelper.h"
#include"Util/DirectoryHelper.h"
#include"Script/ClassProxyHelper.h"
#include"Component/Service/SubService.h"
#include"Component/Rpc/RpcConfigComponent.h"
#include"Component/Scene/ThreadPoolComponent.h"
#include"DB/Redis/RedisClient.h"
namespace Sentry
{
	bool RedisComponent::Awake()
	{
		std::string path;
		this->mRedisConfig.mCount = 3;
		const ServerConfig& config = App::Get()->GetConfig();
		config.GetMember("redis", "count", this->mRedisConfig.mCount);
		config.GetMember("redis", "lua", this->mRedisConfig.mLuaFilePath);
		config.GetMember("redis", "passwd", this->mRedisConfig.mPassword);
		LOG_CHECK_RET_FALSE(config.GetMember("redis", "ip", this->mRedisConfig.mIp));
		LOG_CHECK_RET_FALSE(config.GetMember("redis", "port", this->mRedisConfig.mPort));
		return true;
	}

	std::shared_ptr<RedisResponse>
	RedisComponent::Call(const std::string& func, std::vector<std::string>& args)
	{
		std::string script;
		const size_t pos = func.find('.');
		if (pos == std::string::npos)
		{
			return nullptr;
		}
		std::string tab = func.substr(0, pos);
		if (!this->GetLuaScript(fmt::format("{0}.lua", tab), script))
		{
			LOG_ERROR("not find redis script ", fmt::format("{0}.lua", tab));
			return nullptr;
		}
		std::shared_ptr<RedisRequest> redisCmdRequest(new RedisRequest("EVALSHA"));
		redisCmdRequest->InitParameter(script, (int)args.size() + 1, func.substr(pos + 1));
		for (const std::string& val : args)
		{
			redisCmdRequest->AddParameter(val);
		}
		return this->InvokeCommand(redisCmdRequest);
	}

	std::shared_ptr<RedisResponse> RedisComponent::InvokeCommand(std::shared_ptr<RedisRequest> request)
	{
		std::shared_ptr<RedisClient> redisClient = this->AllotRedisClient();
#ifdef __DEBUG__
		ElapsedTimer elapsedTimer;
		LOG_WARN("redis command name = {0}", redisClient->GetName());
#endif
		if (redisClient == nullptr)
		{
			LOG_ERROR("allot redis client failure");
			return nullptr;
		}
		auto response = redisClient->InvokeCommand(request)->Await();
#ifdef __DEBUG__
		LOG_INFO("invoke redis command use time : {0}ms", elapsedTimer.GetMs());
#endif
		if (!this->mWaitAllotClients.empty())
		{
			auto taskClient = this->mWaitAllotClients.front();
			this->mWaitAllotClients.pop();
			taskClient->SetResult(redisClient);
			return response;
		}
		this->mFreeClients.emplace(redisClient);
		return response;
	}

	bool RedisComponent::LoadLuaScript(const std::string& path)
	{
		std::string content;
		if (!Helper::File::ReadTxtFile(path, content))
		{
			return false;
		}
		auto response = this->InvokeCommand("script", "load", content);
		if (response->HasError())
		{
			LOG_ERROR(response->GetValue());
			return false;
		}
		std::string fileName;
		if (Helper::Directory::GetDirAndFileName(path, content, fileName))
		{
			LOG_WARN(fileName, "  ", response->GetValue());
			this->mLuaCommandMap.emplace(fileName, response->GetValue());
			return true;
		}
		return false;
	}

	bool RedisComponent::GetLuaScript(const std::string& file, std::string& command)
	{
		auto iter = this->mLuaCommandMap.find(file);
		if (iter != this->mLuaCommandMap.end())
		{
			command = iter->second;
			return true;
		}
		return false;
	}

	void RedisComponent::OnStart()
	{
		std::string path;
		std::vector<std::string> luaFiles;
		App::Get()->GetConfig().GetMember("redis", "lua", path);
		Helper::Directory::GetFilePaths(path, "*.lua", luaFiles);

		this->mSubRedisClient = this->MakeRedisClient("Subscribe");
		if (this->mSubRedisClient != nullptr)
		{
			this->SubscribeMessage();
			LOG_DEBUG("subscribe redis client connect successful");
			this->mTaskComponent->Start(&RedisComponent::StartPubSub, this);
		}

		for (size_t index = 0; index < this->mRedisConfig.mCount; index++)
		{
			string name = fmt::format("Command:{0}", index + 1);
			std::shared_ptr<RedisClient> redisClient = this->MakeRedisClient(name);
			if (redisClient == nullptr)
			{
				LOG_ERROR("connect redis {0}:{1} failure", this->mRedisConfig.mIp, this->mRedisConfig.mPort);
				return;
			}
			this->mFreeClients.emplace(redisClient);
		}

		for (const std::string& file : luaFiles)
		{
			LOG_CHECK_RET(this->LoadLuaScript(file));
			LOG_INFO("load redis script {0} successful", file);
		}
#ifdef __DEBUG__
		this->InvokeCommand("FLUSHALL")->IsOk();
#endif
		this->mTaskComponent->Start(&RedisComponent::CheckRedisClient, this);
	}

	std::shared_ptr<RedisClient> RedisComponent::MakeRedisClient(const std::string& name)
	{
#ifdef ONLY_MAIN_THREAD
		IAsioThread& workThread = App::Get()->GetTaskScheduler();
#else
		IAsioThread &workThread = this->mThreadComponent->AllocateNetThread();
#endif
		auto socketProxy = std::make_shared<SocketProxy>(workThread, name);
		auto redisCommandClient = std::make_shared<RedisClient>(socketProxy);

		for (size_t index = 0; index < 3; index++)
		{
			if (redisCommandClient->ConnectAsync(this->mRedisConfig.mIp, this->mRedisConfig.mPort)->Await())
			{
				if (!this->mRedisConfig.mPassword.empty())
				{
					std::shared_ptr<RedisRequest> request(new RedisRequest("AUTH"));
					request->AddParameter(this->mRedisConfig.mPassword);
					auto response = redisCommandClient->InvokeCommand(request)->Await();
					if (!response->IsOk())
					{
						LOG_ERROR("auth redis password error :{0}", this->mRedisConfig.mPassword);
						return nullptr;
					}
				}
				return redisCommandClient;
			}
		}
		return nullptr;
	}

	std::shared_ptr<RedisClient> RedisComponent::AllotRedisClient()
	{
		std::shared_ptr<RedisClient> redisClient;
		if (this->mFreeClients.empty())
		{
			std::shared_ptr<TaskSource<std::shared_ptr<RedisClient>>>
				taskSource(new TaskSource<std::shared_ptr<RedisClient>>());
			this->mWaitAllotClients.push(taskSource);
			redisClient = taskSource->Await();
		}
		else
		{
			redisClient = this->mFreeClients.front();
			this->mFreeClients.pop();
		}

		if (!redisClient->IsOpen())
		{
			const std::string& ip = this->mRedisConfig.mIp;
			unsigned short port = this->mRedisConfig.mPort;
			if (!redisClient->ConnectAsync(ip, port)->Await())
			{
				return nullptr;
			}
			if (!this->mRedisConfig.mPassword.empty())
			{
				std::shared_ptr<RedisRequest> request(new RedisRequest("AUTH"));
				request->AddParameter(this->mRedisConfig.mPassword);
				auto response = redisClient->InvokeCommand(request)->Await();
				if (!response->IsOk())
				{
					LOG_ERROR("auth redis passwork error : ", this->mRedisConfig.mPassword);
					return nullptr;
				}
			}
		}
		return redisClient;
	}

	long long RedisComponent::Publish(const std::string& channel, const std::string& message)
	{
		return this->InvokeCommand("PUBLISH", channel, message)->GetNumber();
	}

	long long RedisComponent::Publish(const std::string& channel, Json::Writer& jsonWriter)
	{
		std::string json = jsonWriter.ToJsonString();
#ifdef __DEBUG__
		LOG_INFO("==== redis publish message ====");
		LOG_INFO("channel = {0}", channel);
		LOG_INFO("message = {0}", json);
#endif
		return this->Publish(channel, json);
	}

	void RedisComponent::CheckRedisClient()
	{

	}

	bool RedisComponent::SubscribeChannel(const std::string& channel)
	{
		std::shared_ptr<RedisRequest> request(new RedisRequest("SUBSCRIBE"));
		request->AddParameter(std::move(channel));
		auto response = this->mSubRedisClient->InvokeCommand(request)->Await();
		return !response->HasError();
	}

	void RedisComponent::SubscribeMessage()
	{
		std::list<SubService*> components;
		App::Get()->GetTypeComponents<SubService>(components);
		for (SubService* subService : components)
		{
			std::vector<std::string> methods;
			subService->GetSubMethods(methods);
			for (const std::string& name : methods)
			{
				const std::string& service = subService->GetName();
				if (this->SubscribeChannel(fmt::format("{0}.{1}", service, name)))
				{
					LOG_INFO("subscribe channel {0}.{1} successful", service, name);
				}
			}
		}
	}

	void RedisComponent::StartPubSub()
	{
		while (this->mSubRedisClient)
		{
			if (!this->mSubRedisClient->IsOpen())
			{
				int count = 0;
				const std::string& ip = this->mRedisConfig.mIp;
				unsigned short port = this->mRedisConfig.mPort;
				while (!this->mSubRedisClient->ConnectAsync(ip, port)->Await())
				{
					LOG_ERROR("connect redis [{0}:{1}] failure count = {2}", ip, port, count++);
					this->mTaskComponent->Sleep(3000);
				}
				if (!this->mRedisConfig.mPassword.empty())
				{
					std::shared_ptr<RedisRequest> request(new RedisRequest("AUTH"));
					request->AddParameter(this->mRedisConfig.mPassword);
					auto response = this->mSubRedisClient->InvokeCommand(request)->Await();
					if (response == nullptr || response->IsOk())
					{
						LOG_FATAL("auth redis passwd error {0}", this->mRedisConfig.mPassword);
						return;
					}
				}
				this->SubscribeMessage();
				LOG_DEBUG("subscribe redis client connect successful");
			}

			auto redisResponse = this->mSubRedisClient->WaitRedisMessage()->Await();
			if (!redisResponse->HasError() && redisResponse->GetArraySize() == 3 &&
				redisResponse->GetValue() == "message")
			{
				const std::string& channel = redisResponse->GetValue(1);
				size_t pos = channel.find('.');
				if (pos != std::string::npos)
				{
					string service = channel.substr(0, pos);
					std::string funcName = channel.substr(pos + 1);
					const std::string& message = redisResponse->GetValue(2);
#ifdef __DEBUG__
					LOG_DEBUG("========= subscribe message =============");
					LOG_DEBUG("channel = {}", channel);
					LOG_DEBUG("message = {}", message);
#endif
					auto subService = this->GetComponent<SubService>(service);
					if (subService != nullptr)
					{
						subService->Publish(funcName, message);
					}
				}
			}
		}
	}

	bool RedisComponent::LateAwake()
	{
		this->mTaskComponent = this->GetComponent<TaskComponent>();
		this->mThreadComponent = this->GetComponent<ThreadPoolComponent>();
		return true;
	}

	long long RedisComponent::AddCounter(const string& key)
	{
		return this->InvokeCommand("INCR", key)->GetNumber();
	}
}
