
LoginComponent = {}
local httpComponent
function LoginComponent.Awake()
    httpComponent = App.GetComponent("HttpComponent")
    return httpComponent ~= nil
end

function LoginComponent.Register(account, passwd, phoneNum)

    local url = "http://127.0.0.1/logic/account/register"
    return httpComponent:Post(url, Json.Encode({
        account = account,
        password = passwd,
        phone_num = phoneNum
    }))
end

function LoginComponent.Login(account, passwd) -- 获取gate地址

    local url = "http://127.0.0.1/logic/account/login"
    local response = httpComponent:Post(url, Json.Encode({
        account = account,
        password = passwd
    }))
    if type(response.data) == "string" then
        return Json.Decode(response.data)
    end
    return nil
end