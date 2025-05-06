//
// Created by zmhy0073 on 2022/10/13.
//

#ifndef APP_TEXTCONFIG_H
#define APP_TEXTCONFIG_H
#include<string>

namespace acs
{
	class ITextConfig
	{
	public:
		virtual bool ReloadConfig() = 0;
		virtual const std::string & GetConfigName() const = 0;
		virtual bool LoadConfig(const std::string & path) = 0;
	public:
		inline const std::string & Path() const { return this->mPath; }
	protected:
		std::string mPath;
	};
}
namespace acs
{
	class TextConfig : public ITextConfig
    {
    public:
		virtual ~TextConfig() = default;
        explicit TextConfig(const char * name) : mName(name), mLastWriteTime(0) { }
    public:
        bool ReloadConfig() final;
		bool LoadConfig(const std::string & path) final;
        const std::string & GetConfigName() const final { return this->mName; }
    protected:
		virtual void OnReload() { }
		virtual void OnLoad(const std::string & path) { }
        virtual bool OnLoadText(const char * str, size_t length) = 0;
        virtual bool OnReloadText(const char * str, size_t length) = 0;
    private:
        std::string mName;
		long long mLastWriteTime;
    };
}

#endif //APP_TEXTCONFIG_H