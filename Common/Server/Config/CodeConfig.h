//
// Created by zmhy0073 on 2022/10/19.
//

#pragma once
#include<memory>
#include<unordered_map>
#include"Core/Singleton/Singleton.h"
#include"Config/Base/CsvTextConfig.h"
namespace acs
{
    class CodeLineConfig
    {
    public:
        int Code;
        std::string Name;
        std::string Desc;
    };

    class CodeConfig : public CsvTextConfig, public ConstSingleton<CodeConfig>
    {
    public:
        CodeConfig() : CsvTextConfig("CodeConfig") { }
    public:
        const std::string & GetDesc(int code) const;
    private:
        bool OnLoadLine(const CsvLineData& lineData) final;
        bool OnReLoadLine(const CsvLineData& lineData) final;
    private:
        std::unordered_map<int, std::unique_ptr<CodeLineConfig>> mConfigs;
    };
}
