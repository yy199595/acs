//
// Created by leyi on 2023/6/6.
//

#ifndef APP_MONGOFACTORY_H
#define APP_MONGOFACTORY_H
#include"MongoProto.h"
namespace Mongo
{
	class MongoFactory
	{
	public:
		static std::shared_ptr<CommandRequest> Insert(const std::string & tab, Bson::Writer::Document & document);
		static std::shared_ptr<CommandRequest> Delete(const std::string & tab, Bson::Writer::Document & document);
		static std::shared_ptr<CommandRequest> Query(const std::string & tab, const std::string & json, int limit = 1);
		static std::shared_ptr<CommandRequest> Update(const std::string & tab, Bson::Writer::Document & select,  Bson::Writer::Document & update, const char * tag, bool upsert);
	private:
		static bool New(const std::string & tab, const char * command, std::shared_ptr<CommandRequest> & request);
	};
}

#endif //APP_MONGOFACTORY_H
