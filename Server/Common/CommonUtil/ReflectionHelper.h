#pragma once
#include<CommonDefine/CommonDef.h>
namespace SoEasy
{
	template<typename Base>
	class ReflectionHelper
	{
	public:
		template<typename T>
		static bool Register(std::string name)
		{
			MapIterator iterator = mRegisterFuncMap.find(name);
			if (iterator == mRegisterFuncMap.end())
			{
				mRegisterFuncMap.insert(std::make_pair(name, Make<T>));
				return true;
			}
			return false;
		}
		static Base * Create(std::string name)
		{
			MapIterator iterator = mRegisterFuncMap.find(name);
			if (iterator != mRegisterFuncMap.end())
			{
				CreateAction & action = iterator->second;
				return action();
			}
			return nullptr;
		}
	private:
		template<typename T>
		static Base * Make()
		{
			return std::make_shared<T>();
		}
	private:
		typedef std::function<std::shared_ptr<Base>(void)> CreateAction;
		static SayNoHashMap<std::string, CreateAction> mRegisterFuncMap;
		typedef typename SayNoHashMap<std::string, CreateAction>::iterator MapIterator;
	};
	template<typename Base>
	SayNoHashMap<std::string, typename ReflectionHelper<Base>::CreateAction> ReflectionHelper<Base>::mRegisterFuncMap;
#define ReflectionRegister(base, type) ReflectionHelper<Module>::Register<LoginModule>(#type)
typedef ReflectionHelper<class Component> ReflectionComponent;
}