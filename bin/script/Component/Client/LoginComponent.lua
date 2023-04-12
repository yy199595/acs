
LoginComponent = {}
local host = "http://127.0.0.1:8080/%s"

function LoginComponent.Register(account, passwd, phoneNum)
    local url = string.format(host, "user/register")
    Log.Info(url)
    local code, response = Http.Post(url, {
        account = account,
        password = passwd,
        phone_num = phoneNum
    })
    if code ~= 200 then
        Log.Error(url, code)
        return nil
    end
    local registerInfo = rapidjson.decode(response)
    if registerInfo == nil then
        return nil
    end
    if registerInfo.code ~= XCode.Successful then
        --Log.Error(response)
    end
    return registerInfo
end

function LoginComponent.Login(account, passwd) -- 获取gate地址
    local url = string.format(host, "user/login")
    local code, response = Http.Post(url, {
        account = account,
        password = passwd
    })
    if code ~= 200 then
        Log.Error(url, code)
        return nil
    end
    local loginInfo = rapidjson.decode(response)
    if loginInfo == nil then
        return nil
    end
    if loginInfo.code ~= XCode.Successful then
        Log.Error(response)
    end
    return loginInfo
end
return LoginComponent