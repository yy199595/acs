//
// Created by yjz on 2022/8/28.
//

#ifndef _MONGOAGENTCOMPONENT_H_
#define _MONGOAGENTCOMPONENT_H_
#include"Component/Component.h"
namespace Sentry
{
	class MongoAgentComponent : public Component
	{
	 public:
		MongoAgentComponent() = default;
		~MongoAgentComponent() = default;
	 public:
		XCode Insert(const Message & message, int index);
		XCode Insert(const char * tab, const Message & message, int index = 0);
		XCode Insert(const char * tab, const std::string & json, int index = 0);
		XCode Remove(const char * tab, const std::string & select, int limit = 1, int index = 0);
		XCode Query(const char * tab, const std::string & select, std::shared_ptr<Message> response);
        XCode Update(const char * tab, const std::string & select, const std::string & data, int index);

    public:
        XCode Save(const Message & message);
        XCode Save(const char * tab, long long id, const std::string & data);
        XCode Save(const char * tab, const std::string & id, const std::string & data);
    private:
		bool LateAwake() final;
	 private:
        db::mongo::update mUpdateRequest;
        db::mongo::insert mInsertRequest;
        db::mongo::remove mRemoveRequest;
        db::mongo::query::request mQueryRequest;
        class MongoService * mMongoService;
	};
}

#endif //_MONGOAGENTCOMPONENT_H_
