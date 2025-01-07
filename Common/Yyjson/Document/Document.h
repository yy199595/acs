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
			bool Push(int v);

			bool Push(bool v);

			bool Push(float v);

			bool Push(double v);

			bool Push(long long v);

			bool Push(const char* val);

			bool Push(const Value& value);

			bool Push(const std::string& v);

			bool PushJson(const std::string& str);

			bool Push(const char* str, size_t size);

		public:
			bool AddNull(const char* key);

			bool Add(const char* k, int v);

			bool Add(const char* k, bool v);

			bool Add(const char* k, char v);

			bool Add(const char* k, size_t v);

			bool Add(const char* k, float value);

			bool Add(const char* k, double value);

			bool Add(const char* k, unsigned int v);

			bool Add(const char* k, const char* val);

			bool Add(const char* k, long long value);

			bool Add(const char* k, const Value& value);

			bool Add(const char* k, yyjson_val* v);

			bool Add(const char* k, yyjson_mut_val* v);

			bool Add(const char* k, const std::string& v);

			bool Add(const char* k, const char* str, size_t size);

			bool Add(const char* k, const std::vector<int>& value);

			bool Add(const char* k, const std::vector<std::string>& value);

			bool AddJson(const char* key, const std::string& json);

			bool AddJson(const char* json, size_t size);

			bool AddJson(const char* key, const char* json, size_t size);

		public:
			std::unique_ptr<Value> AddArray();

			std::unique_ptr<Value> AddObject();

			std::unique_ptr<Value> AddArray(const char* k);

			std::unique_ptr<Value> AddObject(const char* k);

		public:
			inline bool IsInt() const
			{
				return yyjson_mut_is_int(this->mValue);
			}

			inline bool IsArray() const
			{
				return yyjson_mut_is_arr(this->mValue);
			}

			inline bool IsObject() const
			{
				return yyjson_mut_is_obj(this->mValue);
			}

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

			~Document() final
			{
				yyjson_mut_doc_free(this->mDoc);
			}

		public:
			std::string JsonString(bool pretty = false) const;

			bool Encode(std::string* json, bool pretty = false) const;

		private:
		};
	}

	namespace r
	{
		class Value
		{
		public:
			explicit Value(yyjson_val* val) : mValue(val)
			{
			}

			virtual ~Value() = default;

		public:
			bool Get(int& v) const;

			bool Get(bool& v) const;

			bool Get(float& v) const;

			bool Get(long long& v) const;

			bool Get(unsigned int& v) const;

			bool Get(std::string& v) const;

		public:
			int GetInt(const char* k, int defaultVal = 0) const;

		public:
			bool Get(size_t index, int& value) const;

			bool Get(size_t index, std::string& value) const;

			bool Get(size_t index, std::unique_ptr<Value>& value) const;

		public:
			size_t MemberCount() const;

			std::string ToString() const;

			yyjson_val* GetValue()
			{
				return this->mValue;
			}

			size_t GetKeys(std::vector<const char*>& key) const;

		public:
			bool Get(const char* k, int& v) const;

			bool Get(const char* k, char& v) const;

			bool Get(const char* k, bool& v) const;

			bool Get(const char* k, long long& v) const;

			bool Get(const char* k, std::string& v) const;

			bool Get(const char* k, double& v) const;

			bool Get(const char* key, std::vector<int>& value) const;

			bool Get(const char* key, std::unique_ptr<Value>& value) const;

			bool Get(const char* key, std::vector<std::string>& value) const;

		public:
			inline int GetType() const
			{
				return yyjson_get_type(this->mValue);
			}

			inline bool IsArray() const
			{
				return yyjson_is_arr(this->mValue);
			}

			inline bool IsObject() const
			{
				return yyjson_is_obj(this->mValue);
			}

			inline int GetSubType() const
			{
				return yyjson_get_subtype(this->mValue);
			}

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

			bool Decode(const std::string& json);

			bool FromFile(const std::string& path);

			bool Decode(const char* str, size_t size);

			yyjson_doc* GetDoc()
			{
				return this->mDoc;
			}

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
