
LoginComponent = {}
local httpComponent
local host = "http://127.0.0.1:8080/app/web/%s"
function LoginComponent.Awake()
    httpComponent = App.GetComponent("HttpComponent")
    return httpComponent ~= nil
end

function LoginComponent.Register(account, passwd, phoneNum)
    local url = string.format(host, "user/register")
    local response = httpComponent:Post(url, Json.Encode({
        account = account,
        password = passwd,
        phone_num = phoneNum
    }))
    if response.status ~= 200 then
        Log.Error(url, response.error)
        return nil
    end
    return Json.Decode(response.data)
end

function LoginComponent.Login(account, passwd) -- 获取gate地址
    local url = string.format(host, "user/login")
    local response = httpComponent:Post(url, Json.Encode({
        account = account,
        password = passwd
    }))
    if response.status ~= 200 then
        Log.Error(url, response.error)
        return nil
    end
    return Json.Decode(response.data)
end