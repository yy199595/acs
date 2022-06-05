
LoginComponent = {}
local httpComponent
function LoginComponent.Awake()
    httpComponent = App.GetComponent("HttpComponent")
end

function LoginComponent.Register(account, passwd, phoneNum)

    local registerInfo =
    {
        account = account,
        password = passwd,
        phone_num = phoneNum
    }
    local url = "http://127.0.0.1:80/logic/account/register"
    local jsonObject = httpComponent:Post(url, Json.Encode(registerInfo))
    local response = Json.Decode(jsonObject.data)
    if response == nil then
        Log.Error(jsonObject.data)
        return false
    end
    if response.code == XCode.Successful then
        Log.Info("register ", account, " successful")
        return true
    end
    Log.Info("register ", account, " failure")
    return false
end

function LoginComponent.Login(account, passwd) -- 获取gate地址
    local loginInfo =
    {
        account = account,
        password = passwd
    }
    local url = "http://127.0.0.1:80/logic/account/login"
    local jsonObject = httpComponent:Post(url, Json.Encode(loginInfo))
    local response = Json.Decode(jsonObject.data)
    if response == nil then
        Log.Error(jsonObject.data)
        return nil
    end
    if response.code ~= XCode.Successful then
        Log.Error(account, " login failure ")
        return nil
    end
    Log.Info(account, " login successful")
    return response.data
end