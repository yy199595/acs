//
// Created by leyi on 2023/11/17.
//

#ifndef APP_DOCUMENT_H
#define APP_DOCUMENT_H

#include<string>
#include<memory>
#include<vector>
#include"Yyjson/Src/yyjson.h"

namespace json
{

	namespace r
	{
		class Value;
	}
	namespace w
	{
		class Value;
	}

	class IObject
	{
	public:
		virtual bool Encode(json::w::Value & document) = 0;
		virtual bool Decode(const json::r::Value & document) = 0;
	};


	namespace w
	{
		class Value
		{
		public:
			Value(yyjson_mut_doc* doc, yyjson_mut_val* val);

			virtual ~Value() = default;

		protected:
			Value();
		public:
			bool Push(const char *);
			bool Push(const Value& value);
			bool Push(const std::string& v);
			bool PushObject(const std::string& str);
			bool Push(const char* str, size_t size);

			template<typename T>
			inline std::enable_if_t<std::is_base_of<IObject, T>::value, bool> Push(T & object)
			{
				if(!this->IsArray())
				{
					return false;
				}
				auto jsonObject = this->AddObject();
				return object.Encode(*jsonObject);
			}

			template<typename T>
			inline std::enable_if_t<std::is_integral<T>::value, bool> Push(T v)
			{
				if (!this->IsArray())
				{
					return false;
				}
				yyjson_mut_val* val = yyjson_mut_int(this->mDoc, v);
				return yyjson_mut_arr_add_val(this->mValue, val);
			}
			template<typename T>
			inline std::enable_if_t<std::is_floating_point<T>::value, bool> Push(T v)
			{
				if (!this->IsArray())
				{
					return false;
				}
				yyjson_mut_val* val = yyjson_mut_real(this->mDoc, v);
				return yyjson_mut_arr_add_val(this->mValue, val);
			}
		public:
			bool AddNull(const char* key);

			template<typename T>
			inline std::enable_if_t<std::is_integral<T>::value, bool> Add(const char* k, T v)
			{
				if (!this->IsObject())
				{
					return false;
				}
				yyjson_mut_val * val = yyjson_mut_int(this->mDoc, v);
				yyjson_mut_val * key = yyjson_mut_strcpy(this->mDoc, k);
				return yyjson_mut_obj_add(this->mValue, key, val);
			}

			template<typename T>
			inline std::enable_if_t<std::is_enum<T>::value, bool> Add(const char* k, T v)
			{
				if (!this->IsObject())
				{
					return false;
				}
				yyjson_mut_val * val = yyjson_mut_int(this->mDoc, (int)v);
				yyjson_mut_val * key = yyjson_mut_strcpy(this->mDoc, k);
				return yyjson_mut_obj_add(this->mValue, key, val);
			}

			template<typename T>
			inline std::enable_if_t<std::is_floating_point<T>::value, bool> Add(const char* k, T v)
			{
				if (!this->IsObject())
				{
					return false;
				}
				yyjson_mut_val * val = yyjson_mut_real(this->mDoc, v);
				yyjson_mut_val * key = yyjson_mut_strcpy(this->mDoc, k);
				return yyjson_mut_obj_add(this->mValue, key, val);
			}

			bool Add(const char* k, bool v);
			bool Add(const char* k, const char * v);
			bool Add(const char* k, yyjson_val* v);
			bool Add(const char* k, yyjson_mut_val* v);
			bool Add(const char* k, const Value& value);
			bool Add(const char* k, const std::string& v);
			bool Add(const char* k, const char* str, size_t size);

			template<typename T>
			inline std::enable_if_t<std::is_base_of<IObject, T>::value, bool> Add(const char* k, T & value)
			{
				auto jsonObject = this->AddObject(k);
				return value.Encode(*jsonObject);
			}


			template<typename T>
			inline bool Add(const char* k, const std::vector<T>& value)
			{
				if (!this->IsObject())
				{
					return false;
				}
				std::unique_ptr<Value> jsonArray = this->AddArray(k);
				for (const T& val: value)
				{
					jsonArray->Push(val);
				}
			}
		public:
			bool AddObject(const char* json, size_t size);
			bool AddObject(const char* key, const std::string& json);
			bool AddObject(const char* key, const char* json, size_t size);
		public:
			std::unique_ptr<Value> AddArray();
			std::unique_ptr<Value> AddObject();
			std::unique_ptr<Value> AddArray(const char* k);
			std::unique_ptr<Value> AddObject(const char* k);
		public:
			inline bool IsInt() const { return yyjson_mut_is_int(this->mValue);}
			inline bool IsArray() const { return yyjson_mut_is_arr(this->mValue); }
			inline bool IsObject() const { return yyjson_mut_is_obj(this->mValue); }

		protected:
			yyjson_mut_doc* mDoc;
			yyjson_mut_val* mValue;
		};

		class Document final : public Value
		{
		public:
			explicit Document(bool array = false);
			explicit Document(const std::string& json);
			Document(yyjson_mut_doc* doc, yyjson_mut_val* val);
			~Document() final { yyjson_mut_doc_free(this->mDoc); }
		public:
			std::string JsonString(bool pretty = false) const;
			bool Encode(std::string* json, bool pretty = false) const noexcept;
		private:
		};
	}

	namespace r
	{
		class Value
		{
		public:
			explicit Value(yyjson_val* val) : mValue(val) { }

			virtual ~Value() = default;
		public:

			template<typename T>
			inline std::enable_if_t<std::is_integral<T>::value, bool> Get(T& v) const
			{
				if (!yyjson_is_num(this->mValue))
				{
					return false;
				}
				v = yyjson_get_sint(this->mValue);
				return true;
			}

			template<typename T>
			inline std::enable_if_t<std::is_floating_point<T>::value, bool> Get(T& v) const
			{
				if (!yyjson_is_real(this->mValue))
				{
					return false;
				}
				v = yyjson_get_real(this->mValue);
				return true;
			}
		public:
			bool Get(bool& v) const;
			bool Get(std::string& v) const;
			bool Get(size_t index, int& value) const;
			bool Get(size_t index, std::string& value) const;
			int GetInt(const char* k, int defaultVal = 0) const;
			bool Get(size_t index, std::unique_ptr<Value>& value) const;

		public:
			size_t MemberCount() const;
			std::string ToString() const;
			yyjson_val* GetValue() { return this->mValue; }
			std::vector<const char *> GetAllKey() const;
			size_t GetKeys(std::vector<const char*>& key) const;
		public:
			bool Get(const char* k, bool& v) const;
			bool Get(const char* k, std::string& v) const;
			bool Get(const char* key, std::unique_ptr<Value>& value) const;
			bool Get(const char* key, std::vector<std::string>& value) const;

			template<typename T>
			inline std::enable_if_t<std::is_base_of<IObject, T>::value, bool> Get(const char * key, T & value)
			{
				yyjson_val* val = yyjson_obj_get(this->mValue, key);
				if (val == nullptr)
				{
					return false;
				}
				std::unique_ptr<Value> jsonValue = std::make_unique<r::Value>(val);
				return value.Decode(*jsonValue);
			}

			template<typename T>
			inline std::enable_if_t<std::is_integral<T>::value || std::is_enum<T>::value, bool> Get(const char* k, T& v) const
			{
				yyjson_val* val = yyjson_obj_get(this->mValue, k);
				if (!yyjson_is_int(val))
				{
					return false;
				}
				v = (T)yyjson_get_sint(val);
				return true;
			}

			template<typename T>
			inline std::enable_if_t<std::is_floating_point<T>::value, bool> Get(const char* k, T& v) const
			{
				yyjson_val* val = yyjson_obj_get(this->mValue, k);
				if (!yyjson_is_int(val))
				{
					return false;
				}
				v = (T)yyjson_get_num(val);
				return true;
			}

			template<typename T>
			inline std::enable_if_t<std::is_integral<T>::value, bool> Get(const char* key, std::vector<T>& value) const
			{
				yyjson_val* jsonArr = yyjson_obj_get(this->mValue, key);
				if (!yyjson_is_arr(jsonArr))
				{
					return false;
				}
				size_t size = yyjson_arr_size(jsonArr);
				value.reserve(size);
				for (size_t index = 0; index < size; index++)
				{
					yyjson_val* item = yyjson_arr_get(jsonArr, index);
					if (!yyjson_is_num(item))
					{
						return false;
					}
					value.emplace_back((T)yyjson_get_sint(item));
				}
				return true;
			}

			template<typename T>
			inline std::enable_if_t<std::is_base_of<IObject, T>::value, bool> Get(const char* key, std::vector<T>& value) const
			{
				yyjson_val* jsonArr = yyjson_obj_get(this->mValue, key);
				if (!yyjson_is_arr(jsonArr))
				{
					return false;
				}
				size_t size = yyjson_arr_size(jsonArr);
				value.reserve(size);
				for (size_t index = 0; index < size; index++)
				{
					yyjson_val* item = yyjson_arr_get(jsonArr, index);
					if (!yyjson_is_obj(item))
					{
						return false;
					}
					T newItem;
					json::r::Value jsonVal(item);
					if(!newItem.Decode(jsonVal))
					{
						return false;
					}
					value.emplace_back(newItem);
				}
				return true;
			}

			template<typename T>
			inline std::enable_if_t<std::is_floating_point<T>::value, bool> Get(const char* key, std::vector<T>& value) const
			{
				yyjson_val* jsonArr = yyjson_obj_get(this->mValue, key);
				if (!yyjson_is_arr(jsonArr))
				{
					return false;
				}
				size_t size = yyjson_arr_size(jsonArr);
				value.reserve(size);
				for (size_t index = 0; index < size; index++)
				{
					yyjson_val* item = yyjson_arr_get(jsonArr, index);
					if (!yyjson_is_num(item))
					{
						return false;
					}
					value.emplace_back((T)yyjson_get_num(item));
				}
				return true;
			}
		public:
			inline bool IsArray() const { return yyjson_is_arr(this->mValue); }
			inline bool IsObject() const { return yyjson_is_obj(this->mValue); }
			inline int GetType() const { return yyjson_get_type(this->mValue); }
			inline int GetSubType() const { return yyjson_get_subtype(this->mValue); }
		protected:
			yyjson_val* mValue;
		};

		class Document : public Value
		{
		public:
			explicit Document() : Value(nullptr), mDoc(nullptr)
			{
			}

			~Document() override
			{
				yyjson_doc_free(this->mDoc);
			}

		public:
			void SetDoc(yyjson_doc* doc);
			bool Decode(const std::string& json) noexcept;
			bool FromFile(const std::string& path) noexcept;
			bool Decode(const char* str, size_t size) noexcept;
			yyjson_doc* GetDoc() { return this->mDoc; }
			bool DecodeFile(const std::string& path);
		public:
			inline const std::string& GetError()
			{
				return this->mError;
			}

		private:
			yyjson_doc* mDoc;
			std::string mError;
		};
	}

	extern void Merge(json::w::Document& target, json::r::Value& source);
}


#endif //APP_DOCUMENT_H
