//
// Created by leyi on 2023/11/17.
//

#include "Document.h"
#include "Util/Tools/Math.h"
namespace json
{
	w::Value::Value()
	{
		this->mDoc = nullptr;
		this->mValue = nullptr;
	}

	w::Value::Value(yyjson_mut_doc * doc, yyjson_mut_val* val)
			: mDoc(doc), mValue(val)
	{

	}
}

namespace json
{
	bool w::Value::Push(bool v)
	{
		if(!this->IsArray())
		{
			return false;
		}
		yyjson_mut_val * val = yyjson_mut_bool(this->mDoc, v);
		return yyjson_mut_arr_add_val(this->mValue, val);
	}

	bool w::Value::Push(int v)
	{
		return this->Push((long long)v);
	}

	bool w::Value::Push(float v)
	{
		return this->Push((double)v);
	}

	bool w::Value::Push(double v)
	{
		if(!this->IsArray())
		{
			return false;
		}
		yyjson_mut_val * val = yyjson_mut_real(this->mDoc, v);
		return yyjson_mut_arr_add_val(this->mValue, val);
	}

	bool w::Value::Push(long long v)
	{
		if(!this->IsArray())
		{
			return false;
		}
		yyjson_mut_val * val = yyjson_mut_int(this->mDoc, v);
		return yyjson_mut_arr_add_val(this->mValue, val);
	}

	bool w::Value::Push(const char* v)
	{
		if(!this->IsArray())
		{
			return false;
		}
		yyjson_mut_val * val = yyjson_mut_str(this->mDoc, v);
		return yyjson_mut_arr_add_val(this->mValue, val);
	}

	bool w::Value::Push(const std::string& v)
	{
		return this->Push(v.c_str(), v.size());
	}

	bool w::Value::Push(const json::w::Value& v)
	{
		if(!this->IsArray())
		{
			return false;
		}
		return yyjson_mut_arr_add_val(this->mValue, v.mValue);
	}

	bool w::Value::Push(const char* v, size_t size)
	{
		if(!this->IsArray())
		{
			return false;
		}
		yyjson_mut_val * val = yyjson_mut_strncpy(this->mDoc, v, size);
		return yyjson_mut_arr_add_val(this->mValue, val);
	}

	bool w::Value::PushJson(const std::string& json)
	{
		if(!this->IsArray())
		{
			return false;
		}
		yyjson_doc * doc = yyjson_read(json.c_str(), json.size(), YYJSON_WRITE_ALLOW_INVALID_UNICODE);
		if(doc == nullptr)
		{
			return false;
		}

		yyjson_mut_val * obj = yyjson_val_mut_copy(this->mDoc, doc->root);
		bool result = obj != nullptr && yyjson_mut_arr_add_val(this->mValue, obj);
		{
			yyjson_doc_free(doc);
		}
		return result;
	}
}

namespace json
{
	bool w::Value::Add(const char* k, int v)
	{
		return this->Add(k, (long long)v);
	}

	bool w::Value::Add(const char* k, bool v)
	{
		if(!this->IsObject())
		{
			return false;
		}
		return yyjson_mut_obj_add_bool(this->mDoc, this->mValue, k, v);
	}

	bool w::Value::AddNull(const char* key)
	{
		if(!this->IsObject())
		{
			return false;
		}
		return yyjson_mut_obj_add_null(this->mDoc, this->mValue, key);
	}

	bool w::Value::Add(const char* k, char v)
	{
		if(!this->IsObject())
		{
			return false;
		}
		return yyjson_mut_obj_add_int(this->mDoc, this->mValue, k, v);
	}

	bool w::Value::Add(const char* key, size_t v)
	{
		return this->Add(key, (long long)v);
	}

	bool w::Value::Add(const char* key, float value)
	{
		if(!this->IsObject())
		{
			return false;
		}
		return yyjson_mut_obj_add_real(this->mDoc, this->mValue, key, value);
	}

	bool w::Value::Add(const char* key, double value)
	{
		if(!this->IsObject())
		{
			return false;
		}
		return yyjson_mut_obj_add_real(this->mDoc, this->mValue, key, value);
	}

	bool w::Value::Add(const char* k, unsigned int v)
	{
		return this->Add(k, (long long)v);
	}

	bool w::Value::Add(const char* k, const char* v)
	{
		if(!this->IsObject())
		{
			return false;
		}
		return yyjson_mut_obj_add_strcpy(this->mDoc, this->mValue, k, v);
	}

	bool w::Value::Add(const char* k, long long v)
	{
		if(!this->IsObject())
		{
			return false;
		}
		return yyjson_mut_obj_add_int(this->mDoc, this->mValue, k, v);
	}

	bool w::Value::Add(const char* k, const json::w::Value& v)
	{
		if(!this->IsObject())
		{
			return false;
		}
		yyjson_mut_val * key = yyjson_mut_strcpy(this->mDoc, k);
		unsafe_yyjson_mut_obj_add(this->mValue, key, v.mValue, unsafe_yyjson_get_len(this->mValue));
		return true;
	}

	bool w::Value::Add(const char* k, yyjson_mut_val* v)
	{
		yyjson_mut_val * key = yyjson_mut_strcpy(this->mDoc, k);
		yyjson_mut_val * val = yyjson_mut_val_mut_copy(this->mDoc, v);
		return yyjson_mut_obj_add(this->mValue, key, val);
	}

	bool w::Value::Add(const char* k, yyjson_val* v)
	{
		if(!this->IsObject() || v == nullptr)
		{
			return false;
		}
		if(yyjson_is_obj(v))
		{
			std::unique_ptr<json::w::Value> jsonObj = this->AddObject(k);
			{
				yyjson_obj_iter iter;
				yyjson_obj_iter_init(v, &iter);
				while(yyjson_val * k1 = yyjson_obj_iter_next(&iter))
				{
					const char * key = yyjson_get_str(k1);
					jsonObj->Add(key, yyjson_obj_get(v, key));
				}
			}
			return true;
		}
		yyjson_mut_val * value = yyjson_mut_obj_get(this->mValue, k);
		if(value != nullptr)
		{
			yyjson_mut_obj_remove_key(this->mValue, k);
		}
		yyjson_mut_val * key = yyjson_mut_strcpy(this->mDoc, k);
		yyjson_mut_val * val = yyjson_val_mut_copy(this->mDoc, v);
		unsafe_yyjson_mut_obj_add(this->mValue, key, val, unsafe_yyjson_get_len(this->mValue));
		return true;
	}

	bool w::Value::Add(const char* k, const char* v, size_t size)
	{
		if(!this->IsObject())
		{
			return false;
		}
		return yyjson_mut_obj_add_strncpy(this->mDoc, this->mValue, k, v, size);
	}

	bool w::Value::Add(const char* key, const std::vector<std::string> & value)
	{
		if(!this->IsObject())
		{
			return false;
		}
		std::unique_ptr<Value> jsonArray = this->AddArray(key);
		for(const std::string & val : value)
		{
			jsonArray->Push(val);
		}
		return true;
	}

	bool w::Value::Add(const char* key, const std::vector<int> & value)
	{
		if(!this->IsObject())
		{
			return false;
		}
		std::unique_ptr<Value> jsonArray = this->AddArray(key);
		for(const int & val : value)
		{
			jsonArray->Push(val);
		}
		return true;
	}

	bool w::Value::AddJson(const char* k, const std::string& json)
	{
		if(json.empty())
		{
			return false;
		}
		return this->AddJson(k, json.c_str(), json.size());
	}

	bool w::Value::AddJson(const char* json, size_t size)
	{
		if(!this->IsArray())
		{
			return false;
		}
		yyjson_doc * doc = yyjson_read(json, size, YYJSON_WRITE_ALLOW_INVALID_UNICODE);
		if(doc == nullptr)
		{
			return false;
		}
		yyjson_mut_val * obj = yyjson_val_mut_copy(this->mDoc, doc->root);
		bool result = obj && yyjson_mut_arr_add_val(this->mValue, obj);
		{
			yyjson_doc_free(doc);
		}
		return result;
	}

	bool w::Value::AddJson(const char* k, const char* json, size_t size)
	{
		if(!this->IsObject())
		{
			return false;
		}

		yyjson_doc * doc = yyjson_read(json, size, YYJSON_WRITE_ALLOW_INVALID_UNICODE);
		if(doc == nullptr)
		{
			return false;
		}
		yyjson_mut_val * obj = yyjson_val_mut_copy(this->mDoc, doc->root);
		bool result = obj && yyjson_mut_obj_add_val(this->mDoc, this->mValue, k, obj);
		{
			yyjson_doc_free(doc);
		}
		return result;
	}


	bool w::Value::Add(const char* key, const std::string& v)
	{
		return this->Add(key, v.c_str(), v.size());
	}

	std::unique_ptr<w::Value> w::Value::AddArray()
	{
		if(!this->IsArray())
		{
			return nullptr;
		}
		yyjson_mut_val * val = yyjson_mut_arr(this->mDoc);
		if(!yyjson_mut_arr_add_val(this->mValue, val))
		{
			return nullptr;
		}
		return std::make_unique<w::Value>(this->mDoc, val);
	}

	std::unique_ptr<w::Value> w::Value::AddObject()
	{
		if(!this->IsArray())
		{
			return nullptr;
		}
		yyjson_mut_val * val = yyjson_mut_obj(this->mDoc);
		if(!yyjson_mut_arr_add_val(this->mValue, val))
		{
			return nullptr;
		}
		return std::make_unique<w::Value>(this->mDoc, val);
	}

	std::unique_ptr<w::Value> w::Value::AddArray(const char* k)
	{
		if(!this->IsObject())
		{
			return nullptr;
		}

		size_t len = unsafe_yyjson_get_len(this->mValue);
		yyjson_mut_val * val = yyjson_mut_arr(this->mDoc);
		yyjson_mut_val * key = yyjson_mut_strcpy(this->mDoc, k);
		unsafe_yyjson_mut_obj_add(this->mValue, key, val, len);
		return std::make_unique<w::Value>(this->mDoc, val);
	}

	std::unique_ptr<w::Value> w::Value::AddObject(const char* k)
	{
		if(!this->IsObject())
		{
			return nullptr;
		}
		yyjson_mut_val * val = yyjson_mut_obj_get(this->mValue, k);
		if(val == nullptr)
		{
			val = yyjson_mut_obj(this->mDoc);
			yyjson_mut_strcpy(this->mDoc, k);
			yyjson_mut_val * key = yyjson_mut_strcpy(this->mDoc, k);
			unsafe_yyjson_mut_obj_add(this->mValue, key, val, unsafe_yyjson_get_len(this->mValue));
		}
		return std::make_unique<w::Value>(this->mDoc, val);
	}
}

namespace json
{
	namespace w
	{
		Document::Document(bool array)
		{
			this->mDoc = yyjson_mut_doc_new(nullptr);
			this->mValue = array ? yyjson_mut_arr(this->mDoc)
					:  yyjson_mut_obj(this->mDoc);
			yyjson_mut_doc_set_root(this->mDoc, this->mValue);
		}

		Document::Document(const std::string& json)
		{
			yyjson_doc* doc = yyjson_read(json.c_str(), json.size(), YYJSON_WRITE_ALLOW_INVALID_UNICODE);
			if (doc == nullptr)
			{
				return;
			}
			this->mDoc = yyjson_doc_mut_copy(doc, nullptr);
			this->mValue = yyjson_mut_doc_get_root(this->mDoc);
			yyjson_doc_free(doc);
		}

		Document::Document(yyjson_mut_doc* doc, yyjson_mut_val* val)
			: Value(doc, val)
		{

		}

		std::string Document::JsonString(bool pretty)
		{
			std::string result;
			this->Encode(&result, pretty);
			return result;
		}

		bool Document::Encode(std::string* json, bool pretty) const
		{
			size_t data_len;
			bool result = true;
			yyjson_write_err err;
			yyjson_write_flag flag = YYJSON_WRITE_ALLOW_INVALID_UNICODE;
			if (pretty)
			{
				flag = flag | YYJSON_WRITE_PRETTY;
			}
			char * str = yyjson_mut_write(this->mDoc, flag, &data_len);
			if (str == nullptr)
			{
				result = false;
				json->assign(err.msg);
			}
			else
			{
				json->assign(str, data_len);
				free(str);
			}
			return result;
		}
	}
}

namespace json
{
	namespace r
	{
		bool Value::Get(const char* key, std::unique_ptr<r::Value>& value) const
		{
			yyjson_val* val = yyjson_obj_get(this->mValue, key);
			if (val == nullptr)
			{
				return false;
			}
			value = std::make_unique<r::Value>(val);
			return true;
		}

		bool Value::Get(const char* k, bool& v) const
		{
			yyjson_val* val = yyjson_obj_get(this->mValue, k);
			if (!yyjson_is_bool(val))
			{
				return false;
			}
			v = yyjson_get_bool(val);
			return true;
		}

		bool Value::Get(const char* k, int& v) const
		{
			yyjson_val* val = yyjson_obj_get(this->mValue, k);
			if (!yyjson_is_int(val))
			{
				if(yyjson_is_str(val))
				{
					const char * str = yyjson_get_str(val);
					return help::Math::ToNumber(str, v);
				}
				return false;
			}
			v = yyjson_get_int(val);
			return true;
		}

		bool Value::Get(const char* k, char& v) const
		{
			yyjson_val* val = yyjson_obj_get(this->mValue, k);
			if (!yyjson_is_int(val))
			{
				return false;
			}
			v = (char)yyjson_get_int(val);
			return true;
		}

		int Value::GetInt(const char* k, int defaultVal) const
		{
			yyjson_val* val = yyjson_obj_get(this->mValue, k);
			if (!yyjson_is_int(val))
			{
				return defaultVal;
			}
			return yyjson_get_int(val);
		}

		bool Value::Get(const char* k, long long& v) const
		{
			yyjson_val* val = yyjson_obj_get(this->mValue, k);
			if (!yyjson_is_int(val))
			{
				return false;
			}
			v = yyjson_get_sint(val);
			return true;
		}

		bool Value::Get(const char* k, double & v) const
		{
			yyjson_val* val = yyjson_obj_get(this->mValue, k);
			if(!yyjson_is_num(val)) {
				return false;
			}
			v = yyjson_get_num(val);
			return true;
		}

		bool Value::Get(const char* k, std::string& v) const
		{
			yyjson_val* val = yyjson_obj_get(this->mValue, k);
			if (!yyjson_is_str(val))
			{
				return false;
			}
			const char * str = yyjson_get_str(val);
			const size_t size = yyjson_get_len(val);
			v.assign(str, size);
			return true;
		}

		bool Value::Get(const char* k, std::vector<std::string>& value) const
		{
			yyjson_val* jsonArr = yyjson_obj_get(this->mValue, k);
			if (!yyjson_is_arr(jsonArr))
			{
				return false;
			}
			size_t size = yyjson_arr_size(jsonArr);
			for(size_t index = 0; index < size; index++)
			{
				yyjson_val * item = yyjson_arr_get(jsonArr, index);
				if(!yyjson_is_str(item))
				{
					return false;
				}
				const char * str = yyjson_get_str(item);
				const size_t size = yyjson_get_len(item);
				value.emplace_back(str, size);
			}
			return true;
		}

		bool Value::Get(const char* k, std::vector<int>& value) const
		{
			yyjson_val* jsonArr = yyjson_obj_get(this->mValue, k);
			if (!yyjson_is_arr(jsonArr))
			{
				return false;
			}
			size_t size = yyjson_arr_size(jsonArr);
			value.reserve(size);
			for(size_t index = 0; index < size; index++)
			{
				yyjson_val * item = yyjson_arr_get(jsonArr, index);
				if(!yyjson_is_num(item))
				{
					return false;
				}
				value.emplace_back(yyjson_get_int(item));
			}
			return true;
		}
	}
}

namespace json
{
	namespace r
	{
		bool Value::Get(int& v) const
		{
			if (!yyjson_is_int(this->mValue))
			{
				return false;
			}
			v = yyjson_get_int(this->mValue);
			return true;
		}


		bool Value::Get(float& v) const
		{
			if(!yyjson_is_real(this->mValue))
			{
				return false;
			}
			v = (float)yyjson_get_real(this->mValue);
			return true;
		}

		bool Value::Get(unsigned int& v) const
		{
			if(!yyjson_is_uint(this->mValue))
			{
				return false;
			}
			v = (unsigned int)yyjson_get_uint(this->mValue);
			return true;
		}



		bool Value::Get(long long& v) const
		{
			if (!yyjson_is_num(this->mValue))
			{
				return false;
			}
			v = yyjson_get_sint(this->mValue);
			return true;
		}

		bool Value::Get(bool& v) const
		{
			if (!yyjson_is_bool(this->mValue))
			{
				return false;
			}
			v = yyjson_get_bool(this->mValue);
			return true;
		}

		bool Value::Get(std::string& v) const
		{
			if (!yyjson_is_str(this->mValue))
			{
				return false;
			}
			v.assign(yyjson_get_str(this->mValue), yyjson_get_len(this->mValue));
			return true;
		}
	}
	namespace r
	{
		bool Value::Get(size_t index, int& value) const
		{
			if(!yyjson_is_arr(this->mValue))
			{
				return false;
			}
			yyjson_val * val = yyjson_arr_get(this->mValue, index);
			if(val == nullptr || !yyjson_is_int(val))
			{
				return false;
			}
			value = yyjson_get_int(val);
			return true;
		}

		bool Value::Get(size_t index, std::string& value) const
		{
			if(!yyjson_is_arr(this->mValue))
			{
				return false;
			}
			yyjson_val * val = yyjson_arr_get(this->mValue, index);
			if(val == nullptr || !yyjson_is_str(val))
			{
				return false;
			}
			size_t size = yyjson_get_len(val);
			const char * str = yyjson_get_str(val);
			value.assign(str, size);
			return true;
		}

		bool Value::Get(size_t index, std::unique_ptr<Value>& value) const
		{
			if(!yyjson_is_arr(this->mValue))
			{
				return false;
			}
			yyjson_val * val = yyjson_arr_get(this->mValue, index);
			if(val != nullptr)
			{
				value = std::make_unique<Value>(val);
			}
			return val != nullptr && value != nullptr;
		}
	}
	namespace r
	{

		size_t Value::MemberCount() const
		{
			if(this->IsObject())
			{
				return yyjson_obj_size(this->mValue);
			}
			if(this->IsArray())
			{
				return yyjson_arr_size(this->mValue);
			}
			return -1;
		}
		std::string Value::ToString() const
		{
			size_t size = 0;
			std::string result;
			if(yyjson_is_str(this->mValue))
			{
				result.assign(yyjson_get_str(this->mValue));
				return result;
			}
			char * str = yyjson_val_write(this->mValue, YYJSON_WRITE_ALLOW_INVALID_UNICODE, &size);
			if(str != nullptr)
			{
				result.assign(str, size);
			}
			free(str);
			return result;
		}

		size_t Value::GetKeys(std::vector<const char *>& keys) const
		{
			if(!yyjson_is_obj(this->mValue))
			{
				return -1;
			}
			yyjson_obj_iter iter;
			yyjson_obj_iter_init(this->mValue, &iter);
			keys.reserve(yyjson_obj_size(this->mValue));
			while(yyjson_val * k = yyjson_obj_iter_next(&iter))
			{
				keys.emplace_back(yyjson_get_str(k));
			}
			return keys.size();
		}
	}
}

namespace json
{
	namespace r
	{
		bool Document::FromFile(const std::string& path)
		{
			yyjson_read_err readErr;
			if(this->mDoc != nullptr)
			{
				yyjson_doc_free(this->mDoc);
			}
			this->mDoc = yyjson_read_file(path.c_str(), YYJSON_READ_ALLOW_INVALID_UNICODE, nullptr, &readErr);
			if(this->mDoc == nullptr)
			{
				this->mError.assign(readErr.msg);
				return false;
			}
			this->mValue = yyjson_doc_get_root(this->mDoc);
			return true;
		}

		bool Document::Decode(const std::string& json)
		{
			if(json.empty())
			{
				return false;
			}
			return this->Decode(json.c_str(), json.size());
		}



		void Document::SetDoc(yyjson_doc* doc)
		{
			if(this->mDoc != nullptr)
			{
				yyjson_doc_free(this->mDoc);
				this->mDoc = nullptr;
			}
			this->mDoc = doc;
		}

		bool Document::Decode(const char* str, size_t size)
		{
			yyjson_read_err err;
			if(this->mDoc != nullptr)
			{
				yyjson_doc_free(this->mDoc);
				this->mDoc = nullptr;
			}
			this->mDoc = yyjson_read_opts((char*)str, size, YYJSON_READ_ALLOW_INVALID_UNICODE, nullptr, &err);
			if (this->mDoc == nullptr)
			{
				this->mError.assign(err.msg);
				return false;
			}
			this->mValue = yyjson_doc_get_root(this->mDoc);
			return true;
		}

		bool Document::DecodeFile(const std::string& path)
		{
			if(path.empty())
			{
				return false;
			}
			yyjson_read_err err;
			this->mDoc = yyjson_read_file(path.c_str(), YYJSON_READ_ALLOW_INVALID_UNICODE, nullptr, &err);
			if (this->mDoc == nullptr)
			{
				this->mError.assign(err.msg);
				return false;
			}
			this->mValue = yyjson_doc_get_root(this->mDoc);
			return true;
		}
	}
	void Merge(json::w::Document & target, json::r::Value & source)
	{
		std::vector<const char *> keys;
		if(source.GetKeys(keys) > 0)
		{
			for(const char * key : keys)
			{
				std::unique_ptr<json::r::Value> jsonValue;
				if(source.Get(key, jsonValue))
				{
					target.Add(key, jsonValue->GetValue());
				}
			}
		}
	}
}