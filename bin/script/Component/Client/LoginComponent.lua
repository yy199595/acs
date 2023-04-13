
LoginComponent = {}
local host = "http://127.0.0.1:8080/%s"

function LoginComponent.Register(account, passwd, phoneNum)
    local url = string.format(host, "user/register")
    local response = Http.Post(url, {
        account = account,
        password = passwd,
        phone_num = phoneNum
    })
    if response.code ~= 200 then
        Log.Error(response.status)
        return nil
    end
    local message = rapidjson.decode(response.body)
    if message.code ~= XCode.Successful then
        Log.Error(message.error)
        return nil
    end
    return message.data
end

function LoginComponent.Login(account, passwd) -- 获取gate地址
    local url = string.format(host, "user/login")
    local response = Http.Post(url, {
        account = account,
        password = passwd
    })
    table.print(response)
    if response.code ~= 200 then
        Log.Error(response.status)
        return nil
    end
    return rapidjson.decode(response.body)
end
return LoginComponent