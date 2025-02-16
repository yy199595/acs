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

	w::Value::Value(yyjson_mut_doc* doc, yyjson_mut_val* val)
			: mDoc(doc), mValue(val)
	{

	}
}

namespace json
{
	bool w::Value::Push(const char* v)
	{
		if (!this->IsArray())
		{
			return false;
		}
		yyjson_mut_val* val = yyjson_mut_str(this->mDoc, v);
		return yyjson_mut_arr_add_val(this->mValue, val);
	}

	bool w::Value::Push(const std::string& v)
	{
		return this->Push(v.c_str(), v.size());
	}

	bool w::Value::Push(const json::w::Value& v)
	{
		if (!this->IsArray())
		{
			return false;
		}
		return yyjson_mut_arr_add_val(this->mValue, v.mValue);
	}

	bool w::Value::Push(const char* v, size_t size)
	{
		if (!this->IsArray())
		{
			return false;
		}
		yyjson_mut_val* val = yyjson_mut_strncpy(this->mDoc, v, size);
		return yyjson_mut_arr_add_val(this->mValue, val);
	}

	bool w::Value::PushObject(const std::string& json)
	{
		if (!this->IsArray())
		{
			return false;
		}
		yyjson_doc* doc = yyjson_read(json.c_str(), json.size(), YYJSON_WRITE_ALLOW_INVALID_UNICODE);
		if (doc == nullptr)
		{
			return false;
		}

		yyjson_mut_val* obj = yyjson_val_mut_copy(this->mDoc, doc->root);
		bool result = obj != nullptr && yyjson_mut_arr_add_val(this->mValue, obj);
		{
			yyjson_doc_free(doc);
		}
		return result;
	}
}

namespace json
{
	bool w::Value::Add(const char* k, bool v)
	{
		if (!this->IsObject())
		{
			return false;
		}
		yyjson_mut_val * val = yyjson_mut_bool(this->mDoc, v);
		yyjson_mut_val * key = yyjson_mut_strcpy(this->mDoc, k);
		return yyjson_mut_obj_add(this->mValue, key, val);
	}

	bool w::Value::Add(const char* k, const char* v)
	{
		yyjson_mut_val * val = yyjson_mut_strcpy(this->mDoc, v);
		yyjson_mut_val * key = yyjson_mut_strcpy(this->mDoc, k);
		return yyjson_mut_obj_add(this->mValue, key, val);
	}

	bool w::Value::AddNull(const char* k)
	{
		if (!this->IsObject())
		{
			return false;
		}
		yyjson_mut_val * val = yyjson_mut_null(this->mDoc);
		yyjson_mut_val * key = yyjson_mut_strcpy(this->mDoc, k);
		return yyjson_mut_obj_add(this->mValue, key, val);
	}

	bool w::Value::Add(const char* k, const json::w::Value& v)
	{
		if (!this->IsObject())
		{
			return false;
		}
		yyjson_mut_val* key = yyjson_mut_strcpy(this->mDoc, k);
		unsafe_yyjson_mut_obj_add(this->mValue, key, v.mValue, unsafe_yyjson_get_len(this->mValue));
		return true;
	}

	bool w::Value::Add(const char* k, yyjson_mut_val* v)
	{
		yyjson_mut_val* key = yyjson_mut_strcpy(this->mDoc, k);
		yyjson_mut_val* val = yyjson_mut_val_mut_copy(this->mDoc, v);
		return yyjson_mut_obj_add(this->mValue, key, val);
	}

	bool w::Value::Add(const char* k, yyjson_val* v)
	{
		if (!this->IsObject() || v == nullptr)
		{
			return false;
		}
		if (yyjson_is_obj(v))
		{
			std::unique_ptr<json::w::Value> jsonObj = this->AddObject(k);
			{
				yyjson_obj_iter iter;
				yyjson_obj_iter_init(v, &iter);
				while (yyjson_val* k1 = yyjson_obj_iter_next(&iter))
				{
					const char* key = yyjson_get_str(k1);
					jsonObj->Add(key, yyjson_obj_get(v, key));
				}
			}
			return true;
		}
		yyjson_mut_val* value = yyjson_mut_obj_get(this->mValue, k);
		if (value != nullptr)
		{
			yyjson_mut_obj_remove_key(this->mValue, k);
		}
		yyjson_mut_val* key = yyjson_mut_strcpy(this->mDoc, k);
		yyjson_mut_val* val = yyjson_val_mut_copy(this->mDoc, v);
		unsafe_yyjson_mut_obj_add(this->mValue, key, val, unsafe_yyjson_get_len(this->mValue));
		return true;
	}

	bool w::Value::Add(const char* k, const char* v, size_t size)
	{
		if (!this->IsObject())
		{
			return false;
		}
		yyjson_mut_val * key = yyjson_mut_strcpy(this->mDoc, k);
		yyjson_mut_val * val = yyjson_mut_strncpy(this->mDoc, v, size);
		return yyjson_mut_obj_add(this->mValue, key, val);
	}

	bool w::Value::AddObject(const char* k, const std::string& json)
	{
		if (json.empty())
		{
			return false;
		}
		return this->AddObject(k, json.c_str(), json.size());
	}

	bool w::Value::AddObject(const char* json, size_t size)
	{
		if (!this->IsArray())
		{
			return false;
		}
		yyjson_doc* doc = yyjson_read(json, size, YYJSON_WRITE_ALLOW_INVALID_UNICODE);
		if (doc == nullptr)
		{
			return false;
		}
		yyjson_mut_val* obj = yyjson_val_mut_copy(this->mDoc, doc->root);
		bool result = obj && yyjson_mut_arr_add_val(this->mValue, obj);
		{
			yyjson_doc_free(doc);
		}
		return result;
	}

	bool w::Value::AddObject(const char* k, const char* json, size_t size)
	{
		if (!this->IsObject())
		{
			return false;
		}

		yyjson_doc* doc = yyjson_read(json, size, YYJSON_WRITE_ALLOW_INVALID_UNICODE);
		if (doc == nullptr)
		{
			return false;
		}
		yyjson_mut_val* obj = yyjson_val_mut_copy(this->mDoc, doc->root);
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
		if (!this->IsArray())
		{
			return nullptr;
		}
		yyjson_mut_val* val = yyjson_mut_arr(this->mDoc);
		if (!yyjson_mut_arr_add_val(this->mValue, val))
		{
			return nullptr;
		}
		return std::make_unique<w::Value>(this->mDoc, val);
	}

	std::unique_ptr<w::Value> w::Value::AddObject()
	{
		if (!this->IsArray())
		{
			return nullptr;
		}
		yyjson_mut_val* val = yyjson_mut_obj(this->mDoc);
		if (!yyjson_mut_arr_add_val(this->mValue, val))
		{
			return nullptr;
		}
		return std::make_unique<w::Value>(this->mDoc, val);
	}

	std::unique_ptr<w::Value> w::Value::AddArray(const char* k)
	{
		if (!this->IsObject())
		{
			return nullptr;
		}

		size_t len = unsafe_yyjson_get_len(this->mValue);
		yyjson_mut_val* val = yyjson_mut_arr(this->mDoc);
		yyjson_mut_val* key = yyjson_mut_strcpy(this->mDoc, k);
		unsafe_yyjson_mut_obj_add(this->mValue, key, val, len);
		return std::make_unique<w::Value>(this->mDoc, val);
	}

	std::unique_ptr<w::Value> w::Value::AddObject(const char* k)
	{
		if (!this->IsObject())
		{
			return nullptr;
		}
		yyjson_mut_val* val = yyjson_mut_obj_get(this->mValue, k);
		if (val == nullptr)
		{
			val = yyjson_mut_obj(this->mDoc);
			yyjson_mut_strcpy(this->mDoc, k);
			yyjson_mut_val* key = yyjson_mut_strcpy(this->mDoc, k);
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
								 : yyjson_mut_obj(this->mDoc);
			yyjson_mut_doc_set_root(this->mDoc, this->mValue);
		}

		Document::Document(const std::string& json)
		{
			this->mDoc = nullptr;
			this->mValue = nullptr;
			yyjson_doc* doc = yyjson_read(json.c_str(), json.size(), YYJSON_WRITE_ALLOW_INVALID_UNICODE);
			if (doc != nullptr)
			{
				this->mDoc = yyjson_doc_mut_copy(doc, nullptr);
				this->mValue = yyjson_mut_doc_get_root(this->mDoc);
				yyjson_doc_free(doc);
			}
		}

		Document::Document(yyjson_mut_doc* doc, yyjson_mut_val* val)
				: Value(doc, val)
		{

		}

		std::string Document::JsonString(bool pretty) const
		{
			std::string result;
			this->Encode(&result, pretty);
			return result;
		}

		bool Document::Encode(std::string* json, bool pretty) const noexcept
		{
			size_t data_len;
			bool result = true;
			yyjson_write_err err;
			yyjson_write_flag flag = YYJSON_WRITE_ALLOW_INVALID_UNICODE;
			if (pretty)
			{
				flag = flag | YYJSON_WRITE_PRETTY;
			}
			char* str = yyjson_mut_write(this->mDoc, flag, &data_len);
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

		int Value::GetInt(const char* k, int defaultVal) const
		{
			yyjson_val* val = yyjson_obj_get(this->mValue, k);
			if (!yyjson_is_int(val))
			{
				return defaultVal;
			}
			return yyjson_get_int(val);
		}

		bool Value::Get(const char* k, std::string& v) const
		{
			yyjson_val* val = yyjson_obj_get(this->mValue, k);
			if (!yyjson_is_str(val))
			{
				return false;
			}
			const char* str = yyjson_get_str(val);
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
			for (size_t index = 0; index < size; index++)
			{
				yyjson_val* item = yyjson_arr_get(jsonArr, index);
				if (!yyjson_is_str(item))
				{
					return false;
				}
				const char* str = yyjson_get_str(item);
				const size_t size = yyjson_get_len(item);
				value.emplace_back(str, size);
			}
			return true;
		}
	}
}

namespace json
{
	namespace r
	{
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
			if (!yyjson_is_arr(this->mValue))
			{
				return false;
			}
			yyjson_val* val = yyjson_arr_get(this->mValue, index);
			if (val == nullptr || !yyjson_is_int(val))
			{
				return false;
			}
			value = yyjson_get_int(val);
			return true;
		}

		bool Value::Get(size_t index, std::string& value) const
		{
			if (!yyjson_is_arr(this->mValue))
			{
				return false;
			}
			yyjson_val* val = yyjson_arr_get(this->mValue, index);
			if (val == nullptr || !yyjson_is_str(val))
			{
				return false;
			}
			size_t size = yyjson_get_len(val);
			const char* str = yyjson_get_str(val);
			value.assign(str, size);
			return true;
		}

		bool Value::Get(size_t index, std::unique_ptr<Value>& value) const
		{
			if (!yyjson_is_arr(this->mValue))
			{
				return false;
			}
			yyjson_val* val = yyjson_arr_get(this->mValue, index);
			if (val != nullptr)
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
			if (this->IsObject())
			{
				return yyjson_obj_size(this->mValue);
			}
			if (this->IsArray())
			{
				return yyjson_arr_size(this->mValue);
			}
			return -1;
		}

		std::string Value::ToString() const
		{
			size_t size = 0;
			std::string result;
			if (yyjson_is_str(this->mValue))
			{
				result.assign(yyjson_get_str(this->mValue));
				return result;
			}
			char* str = yyjson_val_write(this->mValue, YYJSON_WRITE_ALLOW_INVALID_UNICODE, &size);
			if (str != nullptr)
			{
				result.assign(str, size);
			}
			free(str);
			return result;
		}

		std::vector<const char*> Value::GetAllKey() const
		{
			std::vector<const char *> keys;
			if(yyjson_is_obj(this->mValue))
			{
				yyjson_obj_iter iter;
				yyjson_obj_iter_init(this->mValue, &iter);
				keys.reserve(yyjson_obj_size(this->mValue));
				while (yyjson_val* k = yyjson_obj_iter_next(&iter))
				{
					keys.emplace_back(yyjson_get_str(k));
				}
			}
			return keys;
		}

		size_t Value::GetKeys(std::vector<const char*>& keys) const
		{
			if (!yyjson_is_obj(this->mValue))
			{
				return -1;
			}
			yyjson_obj_iter iter;
			yyjson_obj_iter_init(this->mValue, &iter);
			keys.reserve(yyjson_obj_size(this->mValue));
			while (yyjson_val* k = yyjson_obj_iter_next(&iter))
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
		bool Document::FromFile(const std::string& path) noexcept
		{
			yyjson_read_err readErr;
			if (this->mDoc != nullptr)
			{
				yyjson_doc_free(this->mDoc);
			}
			this->mDoc = yyjson_read_file(path.c_str(), YYJSON_READ_ALLOW_INVALID_UNICODE, nullptr, &readErr);
			if (this->mDoc == nullptr)
			{
				this->mError.assign(readErr.msg);
				return false;
			}
			this->mValue = yyjson_doc_get_root(this->mDoc);
			return true;
		}

		bool Document::Decode(const std::string& json) noexcept
		{
			if (json.empty())
			{
				return false;
			}
			return this->Decode(json.c_str(), json.size());
		}


		void Document::SetDoc(yyjson_doc* doc)
		{
			if (this->mDoc != nullptr)
			{
				yyjson_doc_free(this->mDoc);
				this->mDoc = nullptr;
			}
			this->mDoc = doc;
		}

		bool Document::Decode(const char* str, size_t size) noexcept
		{
			yyjson_read_err err;
			if (this->mDoc != nullptr)
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
			if (path.empty())
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

	void Merge(json::w::Document& target, json::r::Value& source)
	{
		std::vector<const char*> keys;
		if (source.GetKeys(keys) > 0)
		{
			for (const char* key: keys)
			{
				std::unique_ptr<json::r::Value> jsonValue;
				if (source.Get(key, jsonValue))
				{
					target.Add(key, jsonValue->GetValue());
				}
			}
		}
	}
}