//
// Created by zmhy0073 on 2022/11/2.
//

#include"RedisLuaComponent.h"
#include"Util/String/StringHelper.h"
#include"RedisComponent.h"
#include"Util/File/FileHelper.h"
#include"Util/File/DirectoryHelper.h"
#include"Core/System/System.h"
namespace Tendo
{
    bool RedisLuaComponent::LateAwake()
    {
        this->mComponent = this->GetComponent<RedisComponent>();
        std::vector<std::string> luaFiles;
        const RedisClientConfig& config = this->mComponent->Config();
        Helper::Directory::GetFilePaths(config.Script, "*.lua", luaFiles);
        for (const std::string& path : luaFiles)
        {
            std::string content, name;
            if (!Helper::File::ReadTxtFile(path, content))
            {
                LOG_ERROR("load lua [" << path << "] error");
                return false;
            }
            if (!Helper::File::GetFileName(path, name))
            {
                return false;
            }
            std::shared_ptr<RedisResponse> response =
                this->mComponent->SyncRun("SCRIPT", "LOAD", content);
            LOG_CHECK_RET_FALSE(response != nullptr && !response->HasError());
            LOG_CHECK_RET_FALSE(this->OnLoadScript(name, response->GetString()));
        }
        return true;
    }

    bool RedisLuaComponent::OnLoadScript(const std::string &name, const std::string &md5)
    {
        auto iter = this->mLuaMap.find(name);
        if(iter != this->mLuaMap.end())
        {
            return false;
        }
        this->mLuaMap.emplace(name, md5);
        //CONSOLE_LOG_INFO("load redis lua [" << name << "] successful");
        return true;
    }

    std::shared_ptr<RedisRequest> RedisLuaComponent::MakeLuaRequest(const std::string &fullName, const std::string &json)
    {
        size_t pos = fullName.find('.');
        if(pos == std::string::npos)
        {
            return nullptr;
        }
        const std::string tab = fullName.substr(0, pos);
        const std::string func = fullName.substr(pos + 1);
        auto iter = this->mLuaMap.find(tab);
        if(iter == this->mLuaMap.end())
        {
            LOG_ERROR("not find redis script " << fullName);
            return nullptr;
        }
        const std::string & tag = iter->second;
        return RedisRequest::MakeLua(tag, func, json);
    }

    std::shared_ptr<RedisResponse> RedisLuaComponent::Call(const std::string &func, const std::string &json, bool async)
    {
        std::shared_ptr<RedisRequest> request = this->MakeLuaRequest(func, json);
        if(request == nullptr)
        {
            return nullptr;
        }
        std::shared_ptr<RedisResponse> response = async 
            ? this->mComponent->Run(request) : this->mComponent->SyncRun(request);

        if (response != nullptr && response->HasError())
        {
			LOG_ERROR("call [" << func << "] error : " << response->GetString());
        }
        return response;
    }

}