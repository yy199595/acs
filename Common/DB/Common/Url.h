//
// Created by 64658 on 2025/3/15.
//

#ifndef APP_DB_URL_H
#define APP_DB_URL_H
#include <string>
#include <unordered_map>
namespace db
{
	class Url
	{
	public:
		explicit Url(std::string proto);
	public:
		bool Decode(const std::string & url);
		bool Get(const std::string & key, int & value);
		bool Get(const std::string & key, std::string & value);
	private:
		bool Add(const std::string & key, const std::string & value);
	private:
		std::string mProto;
		std::unordered_map<std::string, std::string> mItems;
	};
}

#endif //APP_DB_URL_H
