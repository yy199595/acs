//
// Created by yjz on 2022/1/17.
//

#ifndef GAMEKEEPER_SUBMETHOD_H
#define GAMEKEEPER_SUBMETHOD_H
#include<iostream>
#include"Json/JsonReader.h"
#include"Define/CommonLogDef.h"
#include<google/protobuf/message.h>

namespace Sentry
{
	template<typename T>
	using JsonSubFunction = void (T::*)(const Json::Reader& rapidJsonReader);

	template<typename T, typename T1>
	using ProtoSubFuncrion = void (T::*)(const T& message);
	class SubMethod
	{
	 public:
		virtual void OnPublish(const Json::Reader & jsonReader) = 0;
	};

	template<typename T>
	class JsonSubMethod : public SubMethod
	{
	 public:
		JsonSubMethod(T* obj, JsonSubFunction<T> func)
			: mObj(obj), mFunction(func)
		{
		}

	 public:
		void OnPublish(const Json::Reader & jsonReader) final
		{
			(this->mObj->*mFunction)(jsonReader);
		}
	 private:
		T* mObj;
		JsonSubFunction<T> mFunction;
	};
}
#endif //GAMEKEEPER_SUBMETHOD_H
