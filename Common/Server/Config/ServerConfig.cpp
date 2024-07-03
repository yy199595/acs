
#include"ServerConfig.h"
#include"Core/System/System.h"
#include"Util/File/FileHelper.h"
#include"Log/Common/CommonLogDef.h"
#include"Util/File/DirectoryHelper.h"

namespace joke
{
	ServerConfig::ServerConfig()
		: JsonConfig("ServerConfig")
	{

	}

	bool ServerConfig::OnReLoadJson()
	{
		return true;
	}

	bool ServerConfig::OnLoadText(const char* str, size_t length)
	{
		json::r::Document mainDocument;
		json::w::Document jsonDocument;
		if(!mainDocument.Decode(str, length))
		{
			return false;
		}
		std::string directory;
		const std::string & path = this->Path();
		help::dir::GetDirByPath(path, directory);
		std::unique_ptr<json::r::Value> jsonArray;
		if(mainDocument.Get("include", jsonArray) && jsonArray->IsArray())
		{
			size_t index = 0;
			std::string file;
			while (jsonArray->Get(index, file))
			{
				std::string content;
				json::r::Document document1;
				if(System::ReadFile(fmt::format("{}/{}", directory, file), content))
				{
					if(!document1.Decode(content))
					{
						return false;
					}
					ServerConfig::Append(jsonDocument, document1);
				}
				index++;
			}
		}
		std::string json;
		this->Append(jsonDocument, mainDocument);
		if(jsonDocument.Encode(&json, true) && this->Decode(json))
		{
			//printf("%s\n", json.c_str());
			return this->OnLoadJson();
		}
		return false;
	}

	bool ServerConfig::OnReloadText(const char* str, size_t length)
	{
		return true;
	}

	void ServerConfig::Append(json::w::Value& target, json::r::Value& source)
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

	bool ServerConfig::OnLoadJson()
	{
		std::unique_ptr<json::r::Value> jsonObject;
		if(!this->Get("core", jsonObject))
		{
			return false;
		}
		std::string secret;
		if(jsonObject->Get("secret", secret))
		{
			if(help::fs::FileIsExist(secret))
			{
				help::fs::ReadTxtFile(secret, this->mSecret);
			}
			else
			{
				this->mSecret = secret;
			}
		}
		jsonObject->Get("name", this->mName);
		if(this->Get("path", jsonObject) && jsonObject->IsObject())
		{
			std::vector<const char *> keys;
			if(jsonObject->GetKeys(keys) > 0)
			{
				std::unique_ptr<json::r::Value> value;
				for (const char* k: keys)
				{
					if (!jsonObject->Get(k, value))
					{
						return false;
					}
					this->mPaths.emplace(k, value->ToString());
				}
			}
		}
		System::SetEnv("name", this->mName);
		return true;
	}

	bool ServerConfig::GetPath(const std::string& name, std::string& path) const
	{
		auto iter = this->mPaths.find(name);
		if (iter != this->mPaths.end())
		{
			path = iter->second;
			return true;
		}
		return false;
	}

	std::unique_ptr<json::r::Document> ServerConfig::Read(const std::string& name) const
	{
		std::string path;
		if(!this->GetPath(name, path))
		{
			return nullptr;
		}
		std::unique_ptr<json::r::Document>
		        document = std::make_unique<json::r::Document>();
		return document->FromFile(path) ? std::move(document) : nullptr;
	}
}
