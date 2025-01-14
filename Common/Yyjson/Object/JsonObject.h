//
// Created by 64658 on 2024/8/27.
//

#ifndef APP_JSONOBJECT_H
#define APP_JSONOBJECT_H
#include <unordered_map>
#include "Yyjson/Document/Document.h"
namespace json
{
	class ValueBase
	{
	public:
		virtual bool Set(json::w::Value & document, const std::string & key, void * obj) = 0;
		virtual bool Get(const json::r::Value & document, const std::string & key, void * obj) const = 0;
	};

	template<typename T, typename V>
	class ValueProxy : public ValueBase
	{
	public:
		ValueProxy(V T::*member, bool must) : field(member), mMust(must) { }
	public:
		bool Set(json::w::Value &document, const std::string & key, void *obj) final;
		bool Get(const json::r::Value &document, const std::string & key, void *obj) const final;
	private:
		bool mMust;
		V T::*field;
	};

	template<typename T, typename V>
	class ObjectValueProxy : public ValueBase
	{
	public:
		ObjectValueProxy(V T::*member, bool must) : field(member), mMust(must) { }
	public:
		bool Set(json::w::Value &document, const std::string & key, void *obj) final;
		bool Get(const json::r::Value &document, const std::string & key, void *obj) const final;
	private:
		bool mMust;
		V T::*field;
	};



	template<typename T, typename V>
	class ObjectArrayValueProxy : public ValueBase
	{
	public:
		ObjectArrayValueProxy(std::vector<V> T::*member, bool must) : field(member), mMust(must) { }
	public:
		bool Set(json::w::Value &document, const std::string & key, void *obj) final;
		bool Get(const json::r::Value &document, const std::string & key, void *obj) const final;
	private:
		bool mMust;
		std::vector<V> T::*field;
	};

	template<typename T, typename V>
	inline bool ValueProxy<T, V>::Set(json::w::Value& document, const std::string & key, void* obj)
	{
		T * inst = (T*)obj;
		return document.Add(key.c_str(), inst->*field);
	}

	template<typename T, typename V>
	inline bool ValueProxy<T, V>::Get(const json::r::Value& document, const std::string & key, void* obj) const
	{
		T * inst = (T*)obj;
		return document.Get(key.c_str(), inst->*field) || !this->mMust;
	}

	class IObject
	{
	public:
		virtual bool Decode(const std::string & json) = 0;
		virtual bool Decode(const json::r::Value & document) = 0;
	public:
		virtual bool Encode(std::string & json) = 0;
		virtual bool Encode(json::w::Value & document) = 0;
	};

	template<typename T>
	class Object : public IObject
	{
	public:
		explicit Object() { }
		virtual ~Object() = default;
	public:
		 bool Decode(const std::string & json) final;
		 bool Decode(const json::r::Value & document) final;
	public:
		 bool Encode(std::string & json) final;
		 bool Encode(json::w::Value & document) final;
	public:
		static std::unique_ptr<T> Create(const std::string & json)
		{
			json::r::Document document;
			if(!document.Decode(json))
			{
				return nullptr;
			}
			std::unique_ptr<T> result = std::make_unique<T>();
			{
				if(!result->Decode(document))
				{
					return nullptr;
				}
			}
			return result;
		}
	public:

		template<typename V>
		static std::enable_if_t<std::is_base_of<IObject, V>::value, bool>
			RegisterField(const char * name, std::vector<V> T::*member, bool must = false)
		{
			auto iter = values.find(name);
			if(iter != values.end())
			{
				return false;
			}
			values.emplace(name, new ObjectArrayValueProxy<T, V>(member, must));
			return true;
		}

		template<typename V>
		static std::enable_if_t<!std::is_base_of<IObject, V>::value, bool>
			RegisterField(const char * name, V T::*member, bool must = false)
		{
			auto iter = values.find(name);
			if(iter != values.end())
			{
				return false;
			}
			values.emplace(name, new ValueProxy<T, V>(member, must));
			return true;
		}

		template<typename V>
		static std::enable_if_t<std::is_base_of<IObject, V>::value, bool>
			RegisterField(const char * name, V T::*member, bool must = false)
		{
			auto iter = values.find(name);
			if(iter != values.end())
			{
				return false;
			}
			values.emplace(name, new ObjectValueProxy<T, V>(member, must));
			return true;
		}

	private:
		static std::unordered_map<std::string, ValueBase *> values;
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

	template<typename T>
	inline bool Object<T>::Encode(std::string& json)
	{
		json::w::Document document;
		if(!this->Encode(document))
		{
			return false;
		}
		return document.Encode(&json);
	}

	template<typename T>
	inline bool Object<T>::Decode(const std::string& json)
	{
		json::r::Document document;
		if(!document.Decode(json))
		{
			return false;
		}
		return this->Decode(document);
	}

	template<typename T>
	inline bool Object<T>::Decode(const json::r::Value& document)
	{
		auto iter = values.begin();
		for(; iter != values.end(); iter++)
		{
			const std::string & key = iter->first;
			if(!iter->second->Get(document, key, this))
			{
				return false;
			}
		}
		return true;
	}

	template<typename T>
	inline bool Object<T>::Encode(json::w::Value& document)
	{
		auto iter = values.begin();
		for(; iter != values.end(); iter++)
		{
			const std::string & key = iter->first;
			iter->second->Set(document, key, this);
		}
		return true;
	}

	template<typename T, typename V>
	inline bool ObjectValueProxy<T, V>::Set(json::w::Value& document, const std::string& key, void* obj)
	{
		std::unique_ptr<json::w::Value> jsonValue = document.AddObject(key.c_str());
		{
			T * inst = (T*)obj;
			return (inst->*field).Encode(*jsonValue);
		}
	}

	template<typename T, typename V>
	inline bool ObjectValueProxy<T, V>::Get(const json::r::Value& document, const std::string& key, void* obj) const
	{
		T * inst = (T*)obj;
		std::unique_ptr<json::r::Value> jsonValue;
		if(!document.Get(key.c_str(), jsonValue))
		{
			if(!this->mMust)
			{
				return true;
			}
			return false;
		}
		return (inst->*field).Decode(*jsonValue);
	}


	template<typename T, typename V>
	inline bool ObjectArrayValueProxy<T, V>::Set(json::w::Value& document, const std::string& key, void* obj)
	{
		T * inst = (T*)obj;
		std::unique_ptr<json::w::Value> jsonArray = document.AddArray(key.c_str());
		{
			for(V & val : (inst->*field))
			{
				std::unique_ptr<json::w::Value> jsonObject = jsonArray->AddObject();
				if(!val.Encode(*jsonObject))
				{
					return false;
				}
			}
		}
		return true;
	}

	template<typename T, typename V>
	inline bool ObjectArrayValueProxy<T, V>::Get(const json::r::Value& document, const std::string& key,
			void* obj) const
	{
		T * inst = (T*)obj;
		std::unique_ptr<json::r::Value> jsonArray;
		if(!document.Get(key.c_str(), jsonArray) || !jsonArray->IsArray())
		{
			if(!this->mMust)
			{
				return true;
			}
			return false;
		}
		size_t index = 0;
		std::unique_ptr<json::r::Value> jsonObject;
		while (jsonArray->Get(index, jsonObject))
		{
			index++;
			V value;
			if(!value.Decode(*jsonObject))
			{
				return false;
			}
			(inst->*field).emplace_back(value);
		}
		return true;
	}

	template<typename T>
	std::unordered_map<std::string, ValueBase*> Object<T>::values;

}

#define REGISTER_JSON_CLASS_FIELD(T, field) T::RegisterField(#field, &T::field)

#endif //APP_JSONOBJECT_H
