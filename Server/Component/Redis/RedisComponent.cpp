#include"RedisComponent.h"

#include"App/App.h"
#include"Util/FileHelper.h"
#include"Util/DirectoryHelper.h"
#include"Script/ClassProxyHelper.h"
#include"Component/Service/SubService.h"
#include"Global/RpcConfig.h"
#include"Component/Scene/ThreadPoolComponent.h"
#include"DB/Redis/RedisClient.h"
namespace Sentry
{
	bool RedisComponent::LoadRedisConfig()
	{
		std::string path;
		this->mRedisConfig.mCount = 3;
		const ServerConfig& config = App::Get()->GetConfig();
		this->mRpcAddress = config.GetRpcAddress();
		config.GetMember("redis", "count", this->mRedisConfig.mCount);
		config.GetMember("redis", "lua", this->mRedisConfig.mLuaFilePath);
		config.GetMember("redis", "passwd", this->mRedisConfig.mPassword);
		LOG_CHECK_RET_FALSE(config.GetMember("redis", "ip", this->mRedisConfig.mIp));
		LOG_CHECK_RET_FALSE(config.GetMember("redis", "port", this->mRedisConfig.mPort));
		return true;
	}

	std::shared_ptr<RedisResponse> RedisComponent::Call(const std::string& func, std::vector<std::string>& args)
	{
		std::string script;
		const size_t pos = func.find('.');
		if (pos == std::string::npos)
		{
			return nullptr;
		}
		std::string tab = func.substr(0, pos);
		std::string file = fmt::format("{0}.lua", tab);
		if (!this->GetLuaScript(file, script))
		{
			LOG_ERROR("not find redis script " << file);
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
		LOG_WARN("redis command name = " << request->ToJson());
#endif
		if (redisClient == nullptr)
		{
			LOG_ERROR("allot redis client failure");
			return nullptr;
		}
		std::shared_ptr<RedisResponse> response = redisClient->InvokeCommand(request)->Await();
#ifdef __DEBUG__
		LOG_INFO("invoke redis command use time [" << elapsedTimer.GetMs() << "ms]");
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
		std::shared_ptr<RedisResponse> response = this->InvokeCommand("script", "load", content);
		if (response->HasError())
		{
			LOG_ERROR(response->GetValue());
			return false;
		}
		std::string fileName;
		if (Helper::Directory::GetDirAndFileName(path, content, fileName))
		{
			LOG_WARN(fileName << "  " << response->GetValue());
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

	bool RedisComponent::OnStart()
	{
		std::string path;
		std::vector<std::string> luaFiles;
		App::Get()->GetConfig().GetMember("redis", "lua", path);
		Helper::Directory::GetFilePaths(path, "*.lua", luaFiles);

		std::string address = fmt::format("{0}:{1}",
				this->mRedisConfig.mIp, this->mRedisConfig.mPort);

		this->mSubRedisClient = this->MakeRedisClient("Subscribe");
		if(this->mSubRedisClient == nullptr)
		{
			LOG_ERROR("connect redis " << address << " failure");
			return false;
		}

		for (size_t index = 0; index < this->mRedisConfig.mCount; index++)
		{
			string name = fmt::format("Command:{0}", index + 1);
			std::shared_ptr<RedisClient> redisClient = this->MakeRedisClient(name);
			if (redisClient == nullptr)
			{
				LOG_FATAL("connect redis " << address << " failure");
				return false;
			}
			this->mFreeClients.emplace(redisClient);
		}

		for (const std::string& file : luaFiles)
		{
			LOG_CHECK_RET_FALSE(this->LoadLuaScript(file));
			LOG_INFO("load redis script " << file << " successful");
		}
#ifdef __DEBUG__
		this->InvokeCommand("FLUSHALL")->IsOk();
#endif
		this->SubscribeMessage();
		this->mTaskComponent->Start(&RedisComponent::StartPubSub, this);
		this->mTaskComponent->Start(&RedisComponent::CheckRedisClient, this);
		return true;
	}

	std::shared_ptr<RedisClient> RedisComponent::MakeRedisClient(const std::string& name)
	{
#ifdef ONLY_MAIN_THREAD
		IAsioThread& workThread = App::Get()->GetTaskScheduler();
#else
		IAsioThread& workThread = this->mThreadComponent->AllocateNetThread();
#endif
		std::shared_ptr<SocketProxy> socketProxy = std::make_shared<SocketProxy>(workThread, name);
		std::shared_ptr<RedisClient> redisCommandClient = std::make_shared<RedisClient>(socketProxy);

		for (size_t index = 0; index < 3; index++)
		{
			if (redisCommandClient->ConnectAsync(this->mRedisConfig.mIp, this->mRedisConfig.mPort)->Await())
			{
				if (!this->mRedisConfig.mPassword.empty())
				{
					std::shared_ptr<RedisRequest> request(new RedisRequest("AUTH"));
					request->AddParameter(this->mRedisConfig.mPassword);
					auto responseTask = redisCommandClient->InvokeCommand(request);
					if (!responseTask->Await()->IsOk())
					{
						LOG_ERROR("auth redis password error " << this->mRedisConfig.mPassword);
						return nullptr;
					}
				}
				LOG_INFO("connect redis [" << this->mRedisConfig.mIp << ':' << this->mRedisConfig.mPort << "] successful");
				return redisCommandClient;
			}
			LOG_ERROR("connect redis [" << this->mRedisConfig.mIp << ':' << this->mRedisConfig.mPort << "] failure");
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
					LOG_ERROR("auth redis password error : " << this->mRedisConfig.mPassword);
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

	long long RedisComponent::Publish(const std::string address, const string& func, Json::Writer& jsonWriter)
	{
		jsonWriter.AddMember("func", func);
		return this->Publish(address, jsonWriter);
	}

	long long RedisComponent::Publish(const std::string& channel, Json::Writer& jsonWriter)
	{
		std::string json = jsonWriter.ToJsonString();
#ifdef __DEBUG__
		LOG_INFO("==== redis publish message ====");
		LOG_INFO("channel = " << channel);
		LOG_INFO("message = " << json);
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
		std::vector<std::string> components;
		this->GetApp()->GetComponents(components);
		for(const std::string & name : components)
		{
			SubService * subService = this->GetComponent<SubService>(name);
			if(subService != nullptr && subService->IsStartService())
			{
				std::vector<std::string> methods;
				subService->GetSubMethods(methods);
				for (const std::string& name : methods)
				{
					const std::string& service = subService->GetName();
					if (this->SubscribeChannel(fmt::format("{0}.{1}", service, name)))
					{
						LOG_INFO("subscribe channel [" << service << '.' << name << "] successful");
					}
				}
			}
		}
		this->SubscribeChannel(this->mRpcAddress);
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
					std::string address = fmt::format("{0}:{1}", ip, port);
					LOG_ERROR("connect redis " << address << " failure count = " << count++);
					this->mTaskComponent->Sleep(3000);
				}
				if (!this->mRedisConfig.mPassword.empty())
				{
					std::shared_ptr<RedisRequest> request(new RedisRequest("AUTH"));
					request->AddParameter(this->mRedisConfig.mPassword);
					std::shared_ptr<RedisResponse> response = this->mSubRedisClient->InvokeCommand(request)->Await();
					if (response == nullptr || response->IsOk())
					{
						LOG_FATAL("auth redis passwd error " << this->mRedisConfig.mPassword);
						return;
					}
				}
				this->SubscribeMessage();
				LOG_DEBUG("subscribe redis client connect successful");
			}

			std::shared_ptr<RedisResponse> redisResponse = this->mSubRedisClient->WaitRedisMessage()->Await();
			if (!redisResponse->HasError() && redisResponse->GetArraySize() == 3 && redisResponse->GetValue() == "message")
			{
				const std::string& channel = redisResponse->GetValue(1);
				const std::string& message = redisResponse->GetValue(2);
				std::shared_ptr<Json::Reader> jsonReader(new Json::Reader());

				if (!jsonReader->ParseJson(message))
				{
					LOG_ERROR("parse sub message error");
					continue;
				}

				std::string service;
				std::string funcName;
				if (channel == this->mRpcAddress)
				{
					std::string fullName;
					jsonReader->GetMember("func", fullName);
					size_t pos = fullName.find('.');
					if (pos != std::string::npos)
					{
						service = fullName.substr(0, pos);
						funcName = fullName.substr(pos + 1);
					}
				}
				else
				{
					size_t pos = channel.find('.');
					if (pos != std::string::npos)
					{
						service = channel.substr(0, pos);
						funcName = channel.substr(pos + 1);
					}
				}

#ifdef __DEBUG__
				LOG_DEBUG("========= subscribe message =============");
				LOG_DEBUG("channel = " << channel);
				LOG_DEBUG("message = " << message);
#endif
				SubService* subService = this->GetComponent<SubService>(service);
				if (subService != nullptr)
				{
					subService->Invoke(funcName, *jsonReader);
				}
			}
		}
	}

	bool RedisComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->LoadRedisConfig());
		this->mTaskComponent = this->GetComponent<TaskComponent>();
		this->mThreadComponent = this->GetComponent<ThreadPoolComponent>();
		return true;
	}

	long long RedisComponent::AddCounter(const string& key)
	{
		return this->InvokeCommand("INCR", key)->GetNumber();
	}

}
