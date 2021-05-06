#pragma once
#include<CommonModule/Component.h>
namespace SoEasy
{
	template<typename ObjectType>
	class SayNoFactory
	{
	public:
		template<typename T, typename ... Args>
		void RegisterObject(const std::string & name)
		{
			if (std::is_base_of<ObjectType, T>::value)
			{
				typedef ObjectType*(*Function)(Args ...);
				Function * func = new Function{ SayNoFactory::Create<T, Args ...> };
				funcMap.emplace(name, func);
			}
		}

		template<typename ... Args>
		ObjectType * CreateByName(const std::string & name, Args ... args)
		{
			typedef ObjectType*(*Function)(Args ...);
			if (HasRegisterObject(name))
			{
				Function * func = (Function*)SayNoFactory::funcMap.at(name);
				return (*func)(std::forward<Args>(args)...);
			}
			return nullptr;
		}
		template<typename T, typename ... Args>
		T * CreateByNameCast(const std::string & name, Args ... args)
		{
			if (std::is_base_of<ObjectType, T>::value)
			{
				ObjectType * obj = CreateByName(name, std::forward<Args>(args)...);
				return static_cast<T*>(obj);
			}
			return nullptr;
		}
		bool HasRegisterObject(const std::string & name)
		{
			std::unordered_map<std::string, void *>::iterator iter;
			iter = this->funcMap.find(name);
			return iter != this->funcMap.end();
		}
	private:
		template<typename T, typename ... Args>
		static ObjectType * Create(Args ... args)
		{
			T * data = new T(std::forward<Args>(args)...);
			return static_cast<ObjectType*>(data);
		}
		std::unordered_map<std::string, void *> funcMap;
	};
}
