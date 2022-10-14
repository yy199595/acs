//
// Created by zmhy0073 on 2022/10/13.
//

#ifndef APP_CONFIG_H
#define APP_CONFIG_H
#include<string>
namespace Sentry
{
    class TextConfig
    {
    public:
        TextConfig(const char * name) : mName(name) { }
    public:
        bool ReloadConfig();
        bool LoadConfig(const std::string & path);
        const std::string & GetPath() const { return this->mPath; }
        const std::string & GetName() const { return this->mName; }
    protected:
        virtual bool OnLoadText(const std::string & content) = 0;
        virtual bool OnReloadText(const std::string & content) = 0;
    private:
        std::string mMd5;
        std::string mPath;
        std::string mName;
    };
}
#endif //APP_CONFIG_H