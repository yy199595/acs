//
// Created by leyi on 2023/6/6.
//

#ifndef APP_MONGOFACTORY_H
#define APP_MONGOFACTORY_H
#include"MongoProto.h"

namespace mongo
{
	class MongoFactory
	{
	public:
		static bool New(const std::string & tab, const char * command, std::unique_ptr<Request> & request);
		static std::unique_ptr<Request> Count(const std::string & tab, const json::r::Document & json);
		static std::unique_ptr<Request> Insert(const std::string & tab, bson::Writer::Document & document);
		static std::unique_ptr<Request> Query(const std::string & tab, const json::r::Document & json, int limit = 1, int skip = 0);
		static std::unique_ptr<Request> CreateIndex(const std::string & tab, const std::string & key, int sort = 1, bool unquie = false);
		static std::unique_ptr<Request> Delete(const std::string & tab, bson::Writer::Document & document, int limit = 1);
		static std::unique_ptr<Request> Command(const std::string & tab, const std::string & cmd, const json::r::Document & json);
		static std::unique_ptr<Request> Update(const std::string & tab, bson::Writer::Document & select,  bson::Writer::Document & update, const char * tag, bool upsert, bool multi);
	};
}

#endif //APP_MONGOFACTORY_H
