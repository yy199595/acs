
LoginComponent = {}
local httpComponent
local url = "http://114.115.167.51/app/web/logic/%s"
function LoginComponent.Awake()
    httpComponent = App.GetComponent("HttpComponent")
    return httpComponent ~= nil
end

function LoginComponent.Register(account, passwd, phoneNum)

    return httpComponent:Post(string.format(url, "user/register"), Json.Encode({
        account = account,
        password = passwd,
        phone_num = phoneNum
    }))
end

function LoginComponent.Login(account, passwd) -- 获取gate地址

    local response = httpComponent:Post(string.format(url, "user/login"), Json.Encode({
        account = account,
        password = passwd
    }))
    if type(response.data) == "string" then
        return Json.Decode(response.data)
    end
    return nil
end