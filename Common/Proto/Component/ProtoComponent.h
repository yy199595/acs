//
// Created by zmhy0073 on 2022/6/2.
//

#ifndef SERVER_MESSAGECOMPONENT_H
#define SERVER_MESSAGECOMPONENT_H
#include"Component/Component.h"
#include"google/protobuf/dynamic_message.h"
#include"google/protobuf/compiler/importer.h"
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

    class ProtoComponent final : public Component, public ILuaRegister
    {
    public:
        ProtoComponent();
        ~ProtoComponent() = default;
    public:
        std::shared_ptr<Message> New(const Any & any);
        std::shared_ptr<Message> New(const std::string & name);
        std::shared_ptr<Message> New(const std::string & name, const std::string & json);
		std::shared_ptr<Message> New(const std::string & name, const char * json, size_t size);
	 public:
		bool Write(lua_State * lua, const Message & message);
		std::shared_ptr<Message> Read(lua_State * lua, const std::string & name, int index);
	 private:
        bool LateAwake() final;
		void LoopMessage(const Descriptor * descriptor);
        const Message * FindMessage(const std::string & name);
        void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
	 public:
        bool Load(const char * path);
		bool Import(const char * proto);
	 private:
		std::shared_ptr<compiler::Importer> mImporter;
        std::unordered_map<std::string, std::string> mFiles;
        std::shared_ptr<compiler::DiskSourceTree> mSourceTree;
		std::shared_ptr<DynamicMessageFactory> mDynamicMessageFactory;
		std::unordered_map<std::string, const Message *> mStaticMessageMap;
		std::unordered_map<std::string, const Message *> mDynamicMessageMap;
		std::vector<std::shared_ptr<DynamicMessageFactory>> mDynamicMessageFactorys;
	};
}


#endif //SERVER_MESSAGECOMPONENT_H
