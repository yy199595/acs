
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

    if jsonObject.head.code ~= XCode.Successful then
        Log.Error(account, "register error : ", jsonObject.head.error)
        return false
    end
    Log.Info("register ", account, " successful")

    return Json.Decode(jsonObject.data)
end

function LoginComponent.Login(account, passwd) -- 获取gate地址
    local loginInfo =
    {
        account = account,
        password = passwd
    }
    local url = "http://127.0.0.1:80/logic/account/login"
    local jsonObject = httpComponent:Post(url, Json.Encode(loginInfo))

    if jsonObject.head.code ~= XCode.Successful then
        Log.Error(account, " login error : ", jsonObject.head.error)
        return nil
    end
    Log.Info(account, " login successful")

    return Json.Decode(jsonObject.data)
end