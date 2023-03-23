//
// Created by zmhy0073 on 2022/10/13.
//

#ifndef APP_CONFIG_H
#define APP_CONFIG_H
#include<string>
#include<unordered_map>
namespace Sentry
{
    class TextConfig
    {
    public:
        explicit TextConfig(const char * name) : mName(name) { }
    public:
        bool ReloadConfig();
        bool LoadConfig(const std::string & path);
        const std::string & WorkPath() const;
        const std::string & Path() const { return this->mPath; }
        const std::string & GetName() const { return this->mName; }
    protected:
        virtual bool OnLoadText(const char * str, size_t length) = 0;
        virtual bool OnReloadText(const char * str, size_t length) = 0;
    private:
        std::string mMd5;
        std::string mPath;
        std::string mName;
    };
}

namespace Net
{
    struct Address
    {
    public:
        std::string Ip;
        unsigned short Port;
        std::string FullAddress;
    };
}
#endif //APP_CONFIG_H