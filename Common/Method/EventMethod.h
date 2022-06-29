//
// Created by mac on 2022/5/26.
//

#ifndef SERVER_EVENTMETHOD_H
#define SERVER_EVENTMETHOD_H
#include<memory>
#include"Json/JsonReader.h"

namespace Sentry
{
	template<typename T>
	using EventMethodType1 = bool(T::*)();
	template<typename T>
	using EventMethodType2 = bool(T::*)(const Json::Reader & content);

	class EventMethod
	{
	public:
		explicit EventMethod(const std::string & id)
				: mEveId(id) {}

	public:
		virtual bool Run(std::shared_ptr<Json::Reader> json) = 0;
		const std::string & GetEveId() { return this->mEveId;}
	private:
		const std::string mEveId;
	};

	template<typename T>
	class EventMethod1 final : public EventMethod
	{
	public:
		explicit EventMethod1(const std::string & id, T * o, EventMethodType1<T> methodType1)
				: EventMethod(id), mObj(o), mFunc(methodType1) {}

	public:
		bool Run(std::shared_ptr<Json::Reader> json)
		{
			return (this->mObj->*this->mFunc)();
		}
	private:
		T * mObj;
		EventMethodType1<T> mFunc;
	};

	template<typename T>
	class EventMethod2 final : public EventMethod
	{
	public:
		explicit EventMethod2(const std::string & id, T * o, EventMethodType2<T> methodType)
				: EventMethod(id), mObj(o), mFunc(methodType) {}

	public:
		bool Run(std::shared_ptr<Json::Reader> json)
		{
			return json != nullptr &&  (this->mObj->*this->mFunc)(*json);
		}
	private:
		T * mObj;
		EventMethodType2<T> mFunc;
	};
}

namespace Sentry
{
	class NetEventRegistry
	{
	public:
		NetEventRegistry() = default;
	public:
		template<typename T>
		bool Sub(const std::string & id, EventMethodType1<T> func, T * obj)
		{
			auto iter = this->mEventMethodMap.find(id);
			if(iter != this->mEventMethodMap.end())
			{
				return false;
			}
			this->mEventMethodMap.emplace(id, std::make_shared<EventMethod1<T>>(id, obj, func));
			return true;
		}

		template<typename T>
		bool Sub(const std::string & id, EventMethodType2<T> func, T * obj)
		{
			auto iter = this->mEventMethodMap.find(id);
			if(iter != this->mEventMethodMap.end())
			{
				return false;
			}
			this->mEventMethodMap.emplace(id, std::make_shared<EventMethod2<T>>(id, obj, func));
			return true;
		}
		inline std::shared_ptr<EventMethod> GetEvent(const std::string & eveId);
		const size_t GetEventSize() { return this->mEventMethodMap.size();}
	private:
		std::unordered_map<std::string, std::shared_ptr<EventMethod>> mEventMethodMap;
	};

	inline std::shared_ptr<EventMethod> NetEventRegistry::GetEvent(const std::string& eveId)
	{
		auto iter = this->mEventMethodMap.find(eveId);
		return iter != this->mEventMethodMap.end() ? iter->second : nullptr;
	}
}


#endif //SERVER_EVENTMETHOD_H
