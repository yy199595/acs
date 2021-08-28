#pragma once

#include <Protocol/c2s.pb.h>
#include <Service/LocalService.h>

namespace Sentry
{
    class SceneMysqlComponent;

    class SceneRedisComponent;

    class LoginService : public LocalService
    {
    public:
        LoginService();

        ~LoginService() {}

    protected:
        bool Awake() override;

        void Start() final;

    private:
        XCode Login(long long operId, shared_ptr<c2s::UserVerify_Request> LoginData);

        XCode Register(long long operId, shared_ptr<c2s::UserRegister_Request> registerData);

    private:
        SceneRedisComponent *mRedisManager;
        SceneMysqlComponent *mMysqlManager;
    };
}// namespace Sentry