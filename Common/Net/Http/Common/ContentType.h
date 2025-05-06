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
	class Content;
	class DataType
	{
	public:
		virtual ~DataType() = default;
		virtual std::unique_ptr<Content> New() = 0;
	};
	template<typename T>
	class DataTypeProxy : public DataType
	{
	public:
		inline std::unique_ptr<Content> New() final { return std::make_unique<T>(); }
	};
}

namespace http
{
	class ContentFactory
	{
	public:
		bool New(const std::string & content, std::unique_ptr<Content> & data);
		template<typename T>
		void Add(const std::string & content);
	private:
		std::unordered_map<std::string, std::unique_ptr<http::DataType>> mContentFactoryMap;
	};

	template<typename T>
	void ContentFactory::Add(const std::string& content)
	{
		this->mContentFactoryMap.emplace(content, std::make_unique<http::DataTypeProxy<T>>());
	}
}

#endif //APP_CONTENTTYPE_H
