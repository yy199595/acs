//
// Created by zmhy0073 on 2022/10/13.
//

#ifndef APP_CONFIG_H
#define APP_CONFIG_H
#include<string>
#include<unordered_map>

namespace Tendo
{
	class ITextConfig
	{
	public:
		virtual bool ReloadConfig() = 0;
		virtual const std::string & GetConfigName() const = 0;
		virtual bool LoadConfig(const std::string & path) = 0;
	};
}
namespace Tendo
{
	class TextConfig : public ITextConfig
    {
    public:
        explicit TextConfig(const char * name) : mName(name) { }
    public:
        bool ReloadConfig() final;
        const std::string & WorkPath() const;
		bool LoadConfig(const std::string & path) final;
		const std::string & Path() const { return this->mPath; }
        const std::string & GetConfigName() const final { return this->mName; }
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