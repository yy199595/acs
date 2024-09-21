//
// Created by zmhy0073 on 2022/10/19.
//

#include"CodeConfig.h"
#include"Util/Tools/String.h"
namespace acs
{

    bool CodeConfig::OnLoadLine(const CsvLineData& lineData)
    {
        std::unique_ptr<CodeLineConfig> lineConfig = std::make_unique<CodeLineConfig>();
        {
            lineData.Get("Name", lineConfig->Name);
            lineData.Get("Desc", lineConfig->Desc);
            lineConfig->Code = (int)this->mConfigs.size();
        }
        this->mConfigs.emplace(lineConfig->Code, std::move(lineConfig));
        return true;
    }

    bool CodeConfig::OnReLoadLine(const CsvLineData& lineData)
    {
        return true;
    }
    
    const std::string & CodeConfig::GetDesc(int code) const
    {
        static const std::string unknown("unknown error");
        auto iter = this->mConfigs.find((int)code);
        if(iter != this->mConfigs.end())
        {
            return iter->second->Desc;
        }
        return unknown;
    }
}