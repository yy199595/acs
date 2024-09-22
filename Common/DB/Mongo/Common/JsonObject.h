//
// Created by 64658 on 2024/8/27.
//

#ifndef APP_JSONOBJECT_H
#define APP_JSONOBJECT_H
#include "Yyjson/Document/Document.h"
namespace db
{
	class ValueBase
	{
	public:
		virtual bool Set(json::w::Document & document, void * obj) = 0;
		virtual bool Get(const json::r::Document & document, void * obj) const = 0;
	};

	template<typename T, typename V>
	class ValueProxy : public ValueBase
	{
	public:
		ValueProxy(const char * key, V T::*member) : field(member), key(key) { }
	public:
		bool Set(json::w::Document &document, void *obj) final;
		bool Get(const json::r::Document &document, void *obj) const final;
	private:
		V T::*field;
		std::string key;
	};
	template<typename T, typename V>
	inline bool ValueProxy<T, V>::Set(json::w::Document& document, void* obj)
	{
		T * inst = (T*)obj;
		return document.Add(this->key.c_str(), inst->*field);
	}

	template<typename T, typename V>
	inline bool ValueProxy<T, V>::Get(const json::r::Document& document, void* obj) const
	{
		T * inst = (T*)obj;
		return document.Get(this->key.c_str(), inst->*field);
	}


	class Object
	{
	public:
		explicit Object(const char * db) : mDB(db) { }
	public:
		virtual bool Decode(const std::string & json);
		virtual bool Decode(const json::r::Value & document) = 0;
	public:
		virtual bool Encode(std::string & json) const;
		virtual bool Encode(json::w::Value & document) const = 0;
	public:
		const std::string & GetDBName() const { return this->mDB; }
	public:
		template<typename T, typename V>
		static inline void RegisterField(const char * name, V T::*member);
	private:
		const std::string mDB;
		static std::vector<ValueBase *> values;
	};

	template<typename T>
	static std::unique_ptr<T> Create(const std::string & json)
	{
		json::r::Document document;
		if(!document.Decode(json))
		{
			return nullptr;
		}
		std::unique_ptr<T> data = std::make_unique<T>();
		if(!data->Decode(document))
		{
			return nullptr;
		}
		return data;
	}

	template<typename T, typename V>
	inline void Object::RegisterField(const char* name, V T::*member)
	{
		values.emplace_back(new ValueProxy<T, V>(name, member));
	}

	template<typename T>
	static std::unique_ptr<T> Create(json::r::Value& document)
	{
		std::unique_ptr<T> data = std::make_unique<T>();
		if(!data->Decode(document))
		{
			return nullptr;
		}
		return data;
	}

	inline bool Object::Encode(std::string& json) const
	{
		json::w::Document document;
		if(!this->Encode(document))
		{
			return false;
		}
		return document.Encode(&json);
	}

	inline bool Object::Decode(const std::string& json)
	{
		json::r::Document document;
		if(!document.Decode(json))
		{
			return false;
		}
		return this->Decode(document);
	}
}


#endif //APP_JSONOBJECT_H
