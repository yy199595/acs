//
// Created by 64658 on 2025/6/21.
//

#ifndef APP_FILEFACTORY_H
#define APP_FILEFACTORY_H

#include <string>
#include <unordered_map>
#include "Yyjson/Object/JsonObject.h"
namespace help
{
	class FileFactory
	{
	public:
		bool IsChange(const std::string & path);
		bool Read(const std::string & path, std::string & content);
		bool Read(const std::string & path, json::IObject & document);
		bool Read(const std::string & path, json::r::Document & document);
		bool Read(const std::string & path, std::vector<std::string> & content);
		bool Read(const std::string & path, json::r::Document & document, json::IObject & obj);
	private:
		std::unordered_map<std::string, long long> mFileWriteTimes;
	};
}



#endif //APP_FILEFACTORY_H
