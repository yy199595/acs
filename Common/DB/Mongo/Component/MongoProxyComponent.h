//
// Created by 64658 on 2025/5/12.
//

#ifndef APP_MONGOPROXYCOMPONENT_H
#define APP_MONGOPROXYCOMPONENT_H
#include "Proto/Include/Message.h"
#include "Yyjson/Object/JsonObject.h"
#include "Entity/Component/Component.h"

namespace acs
{
	class MongoProxyComponent final : public Component
	{
	public:
		MongoProxyComponent();
		~MongoProxyComponent() final = default;
	public:
		int Inc(const char * key, int value = 1);
		int Inc(const char * tab, const char * key, int value = 1);
	public:
		int Insert(const char * tab, const pb::Message & document);
		int Insert(const char * tab, const json::w::Value & document);
		int DeleteOne(const char * tab, const json::w::Value & filter);
		int Deletes(const char * tab, const json::w::Value & filter, int limit = 1);
		int SetIndex(const char * tab, const std::string & field, int sort = 1, bool unique = false);
		int UpdateOne(const char * tab, const json::w::Value & filter, const json::w::Value & document);
	public:
		int Count(const char * tab);
		int Count(const char * tab, const json::w::Value & filter);
		int FindOne(const char * tab, const json::w::Value & filter, json::IObject * document);
		int FindOne(const char * tab, const json::w::Value & filter, std::unique_ptr<json::r::Document> & document);
		int FindOne(const char * tab, const json::w::Value & filter, const std::vector<std::string> & fields, std::unique_ptr<json::r::Document> & document);
	private:
		bool LateAwake() final;
	private:
		std::string mReadName;
		std::string mWriteName;
		class NodeComponent * mNodeMgr;
	};
}


#endif //APP_MONGOPROXYCOMPONENT_H
