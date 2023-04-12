//
// Created by mac on 2022/5/26.
//

#ifndef SERVER_EVENTMETHOD_H
#define SERVER_EVENTMETHOD_H
#include<memory>
#include<string>
#include"Entity/Component/Component.h"
namespace Sentry
{
	template<typename T>
	using EventMethodType = void(T::*)(const std::string & content);

	class EventMethod
	{
	public:
		explicit EventMethod(const std::string & id)
				: mEveId(id) {}
	public:
		virtual void Run(const std::string & content) = 0;
	private:
		const std::string mEveId;
	};

	template<typename T>
	class EventMethod1 final : public EventMethod
	{
	public:
		explicit EventMethod1(const std::string & id, T * o, EventMethodType<T> methodType1)
				: EventMethod(id), mObj(o), mFunc(methodType1) {}

	public:
		void Run(const std::string & content)
		{
			(this->mObj->*this->mFunc)(content);
		}
	private:
		T * mObj;
		EventMethodType<T> mFunc;
	};
}

namespace Sentry
{
	class NetEventRegistry
	{
	public:
		NetEventRegistry(Component * component) : mComponent(component) { };
	public:
		template<typename T>
		bool Sub(const std::string & id, EventMethodType<T> func)
		{
			auto iter = this->mEventMethodMap.find(id);
			if(iter != this->mEventMethodMap.end())
			{
				return false;
			}
			T * obj = this->mComponent->Cast<T>();
			std::shared_ptr<EventMethod1<T>> method(new EventMethod1<T>(id, obj, func));
			this->mEventMethodMap.emplace(id, std::move(method));
			return true;
		}

        void GetEvents(std::vector<std::string> & funcs)
        {
            auto iter = this->mEventMethodMap.begin();
            for(; iter != this->mEventMethodMap.end(); iter++)
            {
                funcs.emplace_back(iter->first);
            }
        }

		const size_t GetEventSize() { return this->mEventMethodMap.size();}
		inline std::shared_ptr<EventMethod> GetEvent(const std::string & eveId);

	 private:
		Component * mComponent;
		std::unordered_map<std::string, std::shared_ptr<EventMethod>> mEventMethodMap;
	};

	inline std::shared_ptr<EventMethod> NetEventRegistry::GetEvent(const std::string& eveId)
	{
		auto iter = this->mEventMethodMap.find(eveId);
		return iter != this->mEventMethodMap.end() ? iter->second : nullptr;
	}
}


#endif //SERVER_EVENTMETHOD_H
