//
// Created by zmhy0073 on 2021/11/2.
//

#ifndef GAMEKEEPER_HTTPSERVICEMETHOD_H
#define GAMEKEEPER_HTTPSERVICEMETHOD_H
#include <Network/Http/Http.h>
#include <Network/Http/HttpRemoteSession.h>
namespace GameKeeper
{
    class HttpRemoteRequestHandler;

    template<typename T>
    using HttpServiceMethodType = HttpStatus(T::*)(HttpRemoteRequestHandler *);

    class HttpServiceMethod
    {
    public:
        virtual HttpStatus Invoke(HttpRemoteRequestHandler *handler) = 0;
    };

    template<typename T>
    class HttpServiceMethod1 : public HttpServiceMethod
    {
    public:
        HttpServiceMethod1(T *o, HttpServiceMethodType<T> func)
                : _o(o), _func(func){}

    public:
        HttpStatus Invoke(HttpRemoteRequestHandler *handler) override
        {
            return (_o->*_func)(handler);
        }

    private:
        T *_o;
        HttpServiceMethodType<T> _func;
    };
}
#endif //GAMEKEEPER_HTTPSERVICEMETHOD_H
