#include "Factory.h"
#include<CommonDefine/CommonDef.h>

#include<CommonManager/NetWorkManager.h>
#include<CommonManager/ClientManager.h>
#include<CommonManager/CommandManager.h>
#include<CommonManager/FunctionManager.h>
#include<CommonManager/ServerManager.h>
#include<CommonManager/TimerManager.h>
namespace SoEasy
{
	Type::Type(const std::string name, CreateMgrAction action)
		: mIsComponent(false), mIsManager(true)
	{
		this->nTypeName = name;
		this->mBindCreateMgrAction = action;
	}

	Type::Type(const std::string name, CreateComAction action)
		: mIsComponent(true), mIsManager(false)
	{
		this->nTypeName = name;
		this->mBindCreateComAction = action;
	}

	Manager * Type::NewManager(Applocation * app)
	{
		if (this->mIsManager)
		{
			Manager * manager = this->mBindCreateMgrAction();
			manager->Init(app, this->GetTypeName());
			return manager;
		}
		return nullptr;
	}

	Component * Type::NewComponent(Applocation * app)
	{
		if (this->mIsComponent)
		{
			Component * component = this->mBindCreateComAction();
			component->Init(app, this->GetTypeName());
			return component;
		}
		return nullptr;
	}
}

namespace SoEasy
{
	Manager * Factory::CreateManager(Applocation * app, std::string name)
	{
		Type * type = this->GetType(name);
		return type != nullptr ? type->NewManager(app) : nullptr;
	}

	Component * Factory::CreateComponent(Applocation * app, std::string name)
	{
		Type * type = this->GetType(name);
		return type != nullptr ? type->NewComponent(app) : nullptr;
	}

	Type * Factory::GetType(const std::string name)
	{
		auto iter = this->mRegisterClassNameMap.find(name);
		return iter != this->mRegisterClassNameMap.end() ? iter->second : nullptr;
	}
	Factory * Factory::GetPtr()
	{
		static Factory factory;
		return &factory;
	}

	bool Factory::InitRegisterObject()
	{
		this->RegisterManager<TimerManager>("TimerManager");
		this->RegisterManager<ScriptManager>("ScriptManager");
		this->RegisterManager<ServerManager>("ServerManager");
		this->RegisterManager<ClientManager>("ClientManager");
		this->RegisterManager<CommandManager>("CommandManager");
		this->RegisterManager<NetWorkManager>("NetWorkManager");
		this->RegisterManager<FunctionManager>("FunctionManager");
		this->RegisterManager<CoroutineManager>("CoroutineManager");	

		

	}

	Message * Factory::CreateMessage(const std::string & name)
	{
		if (name.empty())
		{
			return nullptr;
		}
		auto iter = mMessageMap.find(name);
		if (iter != mMessageMap.end())
		{
			Message * pMessage = iter->second;
			pMessage->Clear();
			return pMessage;
		}
		const DescriptorPool * pDescriptorPool = DescriptorPool::generated_pool();
		const Descriptor * pDescriptor = pDescriptorPool->FindMessageTypeByName(name);
		if (pDescriptor != nullptr)
		{
			MessageFactory * factory = MessageFactory::generated_factory();
			Message * pMessage = const_cast<Message *>(factory->GetPrototype(pDescriptor));
			mMessageMap.insert(std::make_pair(name, pMessage));
			return pMessage;
		}
		return nullptr;
	}
	
}
