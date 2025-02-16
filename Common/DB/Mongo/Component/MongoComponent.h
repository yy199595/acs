//
// Created by yjz on 2022/8/28.
//

#ifndef APP_MONGOCOMPONENT_H
#define APP_MONGOCOMPONENT_H
#include"Message/s2s/db.pb.h"
#include"Proto/Include/Message.h"
#include"Entity/Component/Component.h"
#include"Yyjson/Document/Document.h"

namespace mongo
{
	class Batch
	{
	public:
		explicit Batch(const char * tab)
		{
			this->mRequest = std::make_unique<db::mongo::find::request>();
			{
				this->mRequest->set_tab(tab);
			}
		}

		template<typename T>
		inline Batch * AddWhere(const char * field, const std::vector<T> & list)
		{
			json::w::Document filter;
			auto jsonArray = filter.AddObject(field)->AddArray("$in");
			for(const T & value : list)
			{
				jsonArray->Push(value);
			}
			filter.Encode(this->mRequest->mutable_json());
			return this;
		}
		inline Batch * AddField(const char * field)
		{
			this->mRequest->add_fields(field);
			return this;
		}
		inline std::unique_ptr<db::mongo::find::request> Get() {
			return std::move(this->mRequest);
		}
	private:
		std::unique_ptr<db::mongo::find::request> mRequest;
	};
}

namespace acs
{
	class MongoComponent final : public Component
	{
	 public:
		MongoComponent();
		~MongoComponent() final = default;
	 public:
		int Inc(const char * key);
		int Inc(const char * tab, const json::w::Document& filter, const char * field, int value = 1);
	public:
		int Insert(const char * tab, const pb::Message & message);
		int Remove(const char * tab, const std::string & select, int limit);
		int Remove(const char * tab, const json::w::Document & select, int limit);
		int SetIndex(const char * tab, const std::string & field, int sort = 1, bool unique = false);
	public:
		int Insert(const pb::Message & message);
		int Insert(const db::mongo::insert & request);
		int Insert(const char * tab, const std::string & json, bool call = true);
		int Insert(const char * tab, const json::w::Document & json, bool call = true);
	public:
		int Save(const db::mongo::update & request);
		int Update(const db::mongo::update & request);
		int Update(const db::mongo::updates & request);
		int NoWaitUpdate(const db::mongo::update & request);
		int Save(const char * tab, const json::w::Document & select, const json::w::Document & data);
		int Update(const char * tab, const json::w::Document & select, const json::w::Document & data);
	public:
		int Push(const char * tab, const json::w::Document & filter, const json::w::Document & value);
		template<typename T>
		int Push(const char * tab, const json::w::Document & filter, const char * field, const T & value);
	public:
		std::unique_ptr<db::mongo::find::response> Batch(mongo::Batch & batch);
		int Query(const db::mongo::find::request & request, json::w::Value * response);
		int Query(const char * tab, const std::string & select, pb::Message * response);
		int Query(const char * tab, const json::w::Document&, json::w::Value * response);
		int Query(const db::mongo::find::request & request, db::mongo::find::response * response);
		int FindPage(const db::mongo::find_page::request & request, db::mongo::find_page::response * response);
	public:
		int Count(const char * tab, const json::w::Document &);
		int FindOne(const char * tab, const json::w::Document& , std::string * response);
		int FindOne(const db::mongo::find_one::request & request, json::r::Document * response);
		int FindOne(const char * tab, const json::w::Document & select, json::r::Document * response);
		std::unique_ptr<db::mongo::find_one::response> FindOnce(const db::mongo::find_one::request & request);
		std::unique_ptr<db::mongo::find_modify ::response> FindAndModify(const db::mongo::find_modify::request & request);
	public:
		class Server * GetActor();
		std::vector<std::string> GetDatabases();
		std::vector<std::string> GetCollects(const std::string& db = "");
	private:
		std::unique_ptr<db::mongo::find::response> Find(const db::mongo::find::request & request);
		std::unique_ptr<db::mongo::find::response> Find(const char * tab, const std::string & select, int limit = 1);
		std::unique_ptr<db::mongo::find::response> Find(const char * tab, const json::w::Document & select, int limit = 1);
    public:
		bool Awake() final;
		bool LateAwake() final;
	 private:
		std::string mRpc;
		std::string mTmp;
		std::string mServer;
		class ActorComponent * mActComponent;
	};

	template<typename T>
	int MongoComponent::Push(const char * tab, const json::w::Document & filter, const char * field, const T & value)
	{
		json::w::Document document;
		document.Add(field, value);
		return this->Push(tab, filter, document);
	}
}

#endif //APP_MONGOCOMPONENT_H
