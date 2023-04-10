//
// Created by zmhy0073 on 2022/10/19.
//

#pragma once
#include<memory>
#include<unordered_map>
#include"Server/Config/TextConfig.h"
#include"Core/Singleton/Singleton.h"
namespace Tendo
{
    class CodeLineConfig
    {
    public:
        int Code;
        std::string Name;
        std::string Desc;
    };

    class CodeConfig : public TextConfig, public ConstSingleton<CodeConfig>
    {
    public:
        CodeConfig() : TextConfig("CodeConfig") { }
    public:
        const std::string & GetDesc(int code) const;
    private:
        bool OnLoadText(const char *str, size_t length) final;
        bool OnReloadText(const char *str, size_t length) final;
    private:
        std::unordered_map<int, std::unique_ptr<CodeLineConfig>> mConfigs;
    };
}
