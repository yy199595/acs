//
// Created by zmhy0073 on 2021/11/26.
//

#ifndef GAMEKEEPER_JSONSERVICEMETHOD_H
#define GAMEKEEPER_JSONSERVICEMETHOD_H
#include"XCode/XCode.h"
#include"Util/JsonHelper.h"
namespace GameKeeper
{
    template<typename T>
    using JsonServiceMethodType1 = XCode(T::*)();
    template<typename T>
    using JsonServiceMethodType11 = XCode(T::*)(long long);

    template<typename T>
    using JsonServiceMethodType2 = XCode(T::*)(const RapidJsonReader &);
    template<typename T>
    using JsonServiceMethodType22 = XCode(T::*)(long long, const RapidJsonReader &);

    template<typename T>
    using JsonServiceMethodType3 = XCode(T::*)(const RapidJsonReader &, RapidJsonWriter &);
    template<typename T>
    using JsonServiceMethodType33 = XCode(T::*)(long long, const RapidJsonReader &, RapidJsonWriter &);

    class JsonServiceMethod
    {
        public:
            explicit JsonServiceMethod(std::string  name) : mName(std::move(name)) {}
        public:
            virtual bool IsLuaMethod() = 0;
            virtual void SetSocketId(long long id) { };
            virtual XCode Invoke(const RapidJsonReader & request, RapidJsonWriter & response) = 0;
            const std::string & GetName() { return this->mName; }
        private:
            std::string mName;
    };

}

namespace GameKeeper
{
    template<typename T>
    class JsonServiceMethod1 : public JsonServiceMethod
    {
    public:
        JsonServiceMethod1(std::string name, T * o, JsonServiceMethodType1<T> func)
            : JsonServiceMethod(name), _o(o), _func(func),mHasUserId(false) { }
        JsonServiceMethod1(std::string name, T * o, JsonServiceMethodType11<T> func)
                : JsonServiceMethod(name), _o(o), _objfunc(func),mHasUserId(true) { }

    public:
        bool IsLuaMethod() final { return false;}
        void SetSocketId(long long id) final {_o->SetCurSocketId(id); };
        XCode Invoke(const RapidJsonReader & request, RapidJsonWriter & response) final
        {
            if(this->mHasUserId)
            {
                long long userId = 0;
                if(!request.TryGetValue("@user", userId))
                {
                    return XCode::CallArgsError;
                }
                return (_o->*_objfunc)(userId, request);
            }
            return (_o->*_func)(request);
        }
    private:
        T * _o;
        bool mHasUserId;
        JsonServiceMethodType1<T> _func;
        JsonServiceMethodType11<T> _objfunc;
    };
}

namespace GameKeeper
{
    template<typename T>
    class JsonServiceMethod2 : public JsonServiceMethod
    {
    public:
        JsonServiceMethod2(std::string name, T * o, JsonServiceMethodType2<T> func)
                : JsonServiceMethod(name), _o(o), _func(func),mHasUserId(false) { }
        JsonServiceMethod2(std::string name, T * o, JsonServiceMethodType22<T> func)
                : JsonServiceMethod(name), _o(o), _objfunc(func),mHasUserId(true) { }

    public:
        bool IsLuaMethod() final { return false;}
        void SetSocketId(long long id) final {_o->SetCurSocketId(id); };
        XCode Invoke(const RapidJsonReader & request, RapidJsonWriter & response) final
        {
            if(this->mHasUserId)
            {
                long long userId = 0;
                if(!request.TryGetValue("@id", userId))
                {
                    return XCode::CallArgsError;
                }
                return (_o->*_objfunc)(userId, request);
            }
            return (_o->*_func)(request);
        }
    private:
        T * _o;
        bool mHasUserId;
        JsonServiceMethodType2<T> _func;
        JsonServiceMethodType22<T> _objfunc;
    };
}

namespace GameKeeper
{
    template<typename T>
    class JsonServiceMethod3 : public JsonServiceMethod
    {
    public:
        JsonServiceMethod3(std::string name, T * o, JsonServiceMethodType3<T> func)
            : JsonServiceMethod(name), _o(o), _func(func),mHasUserId(false) { }
        JsonServiceMethod3(std::string name, T * o, JsonServiceMethodType33<T> func)
            : JsonServiceMethod(name), _o(o), _objfunc(func),mHasUserId(true) { }

    public:
        bool IsLuaMethod() final { return false;}
        void SetSocketId(long long id) final {_o->SetCurSocketId(id); };
        XCode Invoke(const RapidJsonReader & request, RapidJsonWriter & response) final
        {
            if(this->mHasUserId)
            {
                long long userId = 0;
                if(!request.TryGetValue("@user", userId))
                {
                    return XCode::CallArgsError;
                }
                return (_o->*_objfunc)(userId, request, response);
            }
            return (_o->*_func)(request, response);
        }
    private:
        T * _o;
        const bool mHasUserId;
        JsonServiceMethodType3<T> _func;
        JsonServiceMethodType33<T> _objfunc;
    };
}
#endif //GAMEKEEPER_JSONSERVICEMETHOD_H
