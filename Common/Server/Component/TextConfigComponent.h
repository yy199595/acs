//
// Created by zmhy0073 on 2022/10/13.
//

#ifndef APP_TEXTCONFIGCOMPONENT_H
#define APP_TEXTCONFIGCOMPONENT_H
#include"Config/TextConfig.h"
#include"Component/Component.h"
namespace Sentry
{
    class TextConfigComponent : public Component, public IHotfix
    {
    public:
        TextConfigComponent() = default;
        ~TextConfigComponent() = default;
    public:
        void OnHotFix() final;

        template<typename T>
        const T * GetTextConfig();

        template<typename T>
        const T * GetTextConfig(const std::string & name);

        template<typename T>
        bool LoadTextConfig(const std::string & path);
        bool LoadTextConfig(std::unique_ptr<TextConfig> config, const std::string & path);
    private:
        bool Awake() final;
    private:
        std::unordered_map<size_t, std::string> mKeys;
        std::unordered_map<std::string, std::unique_ptr<TextConfig>> mConfigs;
    };

    template<typename T>
    bool TextConfigComponent::LoadTextConfig(const std::string & path)
    {
        std::unique_ptr<T> config(new T());
        size_t key = typeid(T).hash_code();
        if(this->mKeys.find(key) != this->mKeys.end())
        {
            LOG_ERROR("multiple load [" << config->GetName() << "] type = " << typeid(T).name());
            return false;
        }
        this->mKeys[key] = config->GetName();
        return this->LoadTextConfig(std::move(config), path);
    }
    template<typename T>
    const T * TextConfigComponent::GetTextConfig()
    {
        size_t key = typeid(T).hash_code();
        auto iter = this->mKeys.find(key);
        if(iter == this->mKeys.end())
        {
            return nullptr;
        }
        auto iter1 = this->mConfigs.find(iter->second);
        return iter1 != this->mConfigs.end() ?  static_cast<T*>(iter1->second.get()) : nullptr;
    }

    template<typename T>
    const T *TextConfigComponent::GetTextConfig(const std::string &name)
    {
        auto iter1 = this->mConfigs.find(name);
        return iter1 != this->mConfigs.end() ?  static_cast<T*>(iter1->second.get()) : nullptr;
    }
}


#endif //APP_TEXTCONFIGCOMPONENT_H
