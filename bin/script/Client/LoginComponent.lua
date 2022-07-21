
LoginComponent = {}
local httpComponent
function LoginComponent.Awake()
    httpComponent = App.GetComponent("HttpComponent")
    return httpComponent ~= nil
end

function LoginComponent.Register(account, passwd, phoneNum)

    local registerInfo =
    {
        account = account,
        password = passwd,
        phone_num = phoneNum
    }
    local url = "http://127.0.0.1:80/logic/account/register"
    return httpComponent:Post(url, Json.Encode(registerInfo))
end

function LoginComponent.Login(account, passwd) -- 获取gate地址
    local loginInfo =
    {
        account = account,
        password = passwd
    }
    local url = "http://127.0.0.1:80/logic/account/login"
    return httpComponent:Post(url, Json.Encode(loginInfo))
end