//
// Created by yjz on 2022/1/17.
//

#ifndef GAMEKEEPER_SUBMETHOD_H
#define GAMEKEEPER_SUBMETHOD_H
#include<iostream>
#include"Json/JsonReader.h"
#include"Json/JsonWriter.h"
#include"Define/CommonLogDef.h"
#include<google/protobuf/message.h>

namespace Sentry
{
	template<typename T>
	using JsonSubFunction = XCode(T::*)(const Json::Reader& rapidJsonReader);

	template<typename T>
	using JsonSubFunction2 = XCode(T::*)(const Json::Reader& rapidJsonReader, Json::Writer & jsonWriter);

	class SubMethod
	{
	 public:
		virtual XCode OnPublish(const Json::Reader & jsonReader, Json::Writer & response) = 0;
	};

	template<typename T>
	class JsonSubMethod : public SubMethod
	{
	 public:
		JsonSubMethod(T* obj, JsonSubFunction<T> func)
			: mObj(obj), mFunction(func) { }
	 public:
		XCode OnPublish(const Json::Reader & jsonReader, Json::Writer & response) final
		{
			return (this->mObj->*mFunction)(jsonReader);
		}
	 private:
		T* mObj;
		JsonSubFunction<T> mFunction;
	};

	template<typename T>
	class JsonSubMethod2 : public SubMethod
	{
	public:
		JsonSubMethod2(T* obj, JsonSubFunction2<T> func)
				: mObj(obj), mFunction(func) { }
	public:
		XCode OnPublish(const Json::Reader &jsonReader, Json::Writer &response)final
		{
			return (this->mObj->*mFunction)(jsonReader, response);
		}
	private:
		T* mObj;
		JsonSubFunction2<T> mFunction;
	};
}
#endif //GAMEKEEPER_SUBMETHOD_H
