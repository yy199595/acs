//
// Created by zmhy0073 on 2022/6/2.
//

#ifndef SERVER_MESSAGECOMPONENT_H
#define SERVER_MESSAGECOMPONENT_H
#include"Component/Component.h"
#include<google/protobuf/compiler/importer.h>
namespace Sentry
{

    class ImportError : public compiler::MultiFileErrorCollector
    {
    public:
        ImportError();
    public:
        bool HasError() const { return this->mHasError;}
        bool HasWarning() const { return this->mHasWarning;}
    protected:
        void AddError(const std::string &filename, int line, int column, const std::string &message) final;
        void AddWarning(const std::string &filename, int line, int column, const std::string &message) final;

    private:
        bool mHasError;
        bool mHasWarning;
    };

    class MessageComponent final : public Component, public ILuaRegister
    {
    public:
        MessageComponent() = default;
        ~MessageComponent() = default;
    public:
        std::shared_ptr<Message> New(const Any & any);
        std::shared_ptr<Message> New(const std::string & name);
        std::shared_ptr<Message> New(const std::string & name, const std::string & json);
    private:
        const Message * FindMessage(const std::string & name);
        void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
        bool Load(const std::string path, std::vector<std::string> protos);
    private:
        std::unordered_map<std::string, const Message *> mMessageMap;
        std::unordered_map<std::string, const FileDescriptor *> mFileDescripors;
    };
}


#endif //SERVER_MESSAGECOMPONENT_H
