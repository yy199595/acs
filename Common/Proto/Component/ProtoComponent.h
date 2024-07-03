//
// Created by zmhy0073 on 2022/6/2.
//

#ifndef APP_PROTOCOMPONENT_H
#define APP_PROTOCOMPONENT_H

#include"Proto/Include/Message.h"
#include"google/protobuf/any.pb.h"
#include"Entity/Component/Component.h"
#include"google/protobuf/dynamic_message.h"
#include"google/protobuf/compiler/importer.h"

struct lua_State;
namespace joke
{

	class ImportError : public pb::compiler::MultiFileErrorCollector
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

	class ProtoComponent final : public Component, public ILuaRegister, public IHotfix
    {
    public:
        ProtoComponent();
        ~ProtoComponent() override = default;
    public:
		bool Load(const char * path);
		pb::Message * Temp(const std::string & name);
		bool Import(const char * proto, std::vector<std::string> & types);
		bool New(const pb::Any & any, std::unique_ptr<pb::Message> & message);
		bool New(const std::string & name, std::unique_ptr<pb::Message> & message);
		bool New(const std::string & name, const std::string & json, std::unique_ptr<pb::Message> & message);
		bool New(const std::string & name, const char * json, size_t size, std::unique_ptr<pb::Message> & message);
	 public:
		bool Write(lua_State * lua, const pb::Message & message);
		pb::Message * Read(lua_State * lua, const std::string & name, int index);
	 private:
		bool OnHotFix() final;
		bool LateAwake() final;
		void OnLuaRegister(Lua::ModuleClass &luaRegister) final;
        const pb::Message * FindMessage(const std::string & name);
		void LoopMessage(const pb::Descriptor * descriptor, std::vector<std::string> & protos);
	private:
		std::unique_ptr<ImportError> mImportError;
		std::unique_ptr<pb::compiler::Importer> mImporter;
        std::unordered_map<std::string, long long> mFiles;
        std::unique_ptr<pb::compiler::DiskSourceTree> mSourceTree;
		std::unordered_map<std::string, pb::Message *> mTempMessages;
		std::unique_ptr<pb::DynamicMessageFactory> mDynamicMessageFactory;
		std::unordered_map<std::string, const pb::Message *> mStaticMessageMap;
		std::unordered_map<std::string, const pb::Message *> mDynamicMessageMap;
		std::vector<std::unique_ptr<pb::DynamicMessageFactory>> mDynamicMessageFactorys;
	};
}


#endif //APP_PROTOCOMPONENT_H
