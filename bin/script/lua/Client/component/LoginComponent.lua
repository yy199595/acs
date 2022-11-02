
LoginComponent = {}
local httpComponent
local host = "http://127.0.0.1:8080/%s"
function LoginComponent.Awake()
    httpComponent = App.GetComponent("HttpComponent")
    return httpComponent ~= nil
end

function LoginComponent.Register(account, passwd, phoneNum)
    local url = string.format(host, "user/register")
    local code, response = httpComponent:Post(url, {
        account = account,
        password = passwd,
        phone_num = phoneNum
    })
    if code ~= 200 then
        Log.Error(url, code)
        return nil
    end
    return Json.Decode(response)
end

function LoginComponent.Login(account, passwd) -- 获取gate地址
    local url = string.format(host, "user/login")
    local code, response = httpComponent:Post(url, {
        account = account,
        password = passwd
    })
    if code ~= 200 then
        Log.Error(url, code)
        return nil
    end
    print(response)
    return Json.Decode(response)
end
return LoginComponent