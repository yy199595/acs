//
// Created by yjz on 2022/1/17.
//

#ifndef GAMEKEEPER_SUBMETHOD_H
#define GAMEKEEPER_SUBMETHOD_H
#include<iostream>
#include"rapidjson/document.h"
#include"Define/CommonLogDef.h"
#include<google/protobuf/message.h>

namespace GameKeeper
{
    template<typename T>
    using JsonSubFunction = void(T::*)(const rapidjson::Document & document);

    template<typename T, typename T1>
    using ProtoSubFuncrion = void(T::*)(const T & message);
    class SubMethod
    {
    public:
        virtual void OnPublish(const std::string & message) = 0;
    };

    template<typename T>
    class JsonSubMethod : public SubMethod
    {
    public:
        JsonSubMethod(T * obj, JsonSubFunction<T> func)
            : mObj(obj), mFunction(func) { }

    public:
        void OnPublish(const std::string &message) final
        {
            rapidjson::Document document;
            document.Parse(message.c_str(), message.size());
            if(document.HasParseError())
            {
                std::cerr << "parse json error : json = " << message << std::endl;
                return;
            }
            (this->mObj->*mFunction)(document);
        }
    private:
        T * mObj;
        JsonSubFunction<T> mFunction;
    };

    template<typename T, typename T1>
    class ProtoSubMethod : public SubMethod
    {
    public:
        ProtoSubMethod(T * obj, ProtoSubFuncrion<T, T1> func)
            : mObj(obj), mFunction(func) {}
    public:
        void OnPublish(const std::string &message) final
        {
            T data;
            if(!data.ParseFromString(message))
            {
                std::cerr << "parse proto error " << std::endl;
                return;
            }
            (this->mObj->*mFunction)(data);
        }
    private:
        T * mObj;
        ProtoSubFuncrion<T, T1> mFunction;
    };
}
#endif //GAMEKEEPER_SUBMETHOD_H
