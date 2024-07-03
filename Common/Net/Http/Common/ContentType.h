//
// Created by leyi on 2023/11/20.
//

#ifndef APP_CONTENTTYPE_H
#define APP_CONTENTTYPE_H
#include <memory>
#include <string>
#include"Core/Map/HashMap.h"
#include"Core/Singleton/Singleton.h"
namespace http
{
	class Data;
	class DataType
	{
	public:
		virtual std::unique_ptr<Data> New() = 0;
	};
	template<typename T>
	class DataTypeProxy : public DataType
	{
	public:
		std::unique_ptr<Data> New() { return std::make_unique<T>(); }
	};
}

namespace http
{
	class ContentFactory
	{
	public:
		bool New(const std::string & content, std::unique_ptr<Data> & data);
		template<typename T>
		void Add(const std::string & content);
	private:
		std::unordered_map<std::string, http::DataType *> mContentFactoryMap;
	};

	template<typename T>
	void ContentFactory::Add(const std::string& content)
	{
		this->mContentFactoryMap.emplace(content, new http::DataTypeProxy<T>());
	}
}

#endif //APP_CONTENTTYPE_H
