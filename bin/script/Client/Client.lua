
local Main = {}

local phoneNum = 13716061995
local account = tostring(os.time()) .. "@qq.com"
local password = tostring(os.time()) .. "#123456"
local loginComponent = require("component.LoginComponent")

local CallMongo = function()
    local t1 = Time.NowMilTime()
    local code, res = Client.Call("MongoService.Query", {
        tab = "user.account",
        json = rapidjson.encode({
            _id = "646585122@qq.com"
        }),
        limit = 1
    })
    table.print(res)
    local t = Time.NowMilTime() - t1
    Console.Warn("cal MongoService.Query [", t, "]ms");
end

local CallChat = function()
    local t1 = Time.NowMilTime()
    local code = Client.Call("ChatService.Ping", {
        user_id = 1122, msg_type = 1, message = "hello"
    })
    if code == XCode.Successful then

    end
    local t = Time.NowMilTime() - t1
    Console.Error("cal ChatService.Ping [" .. t .. "]ms");
end

local TestHttp = function()
    local count = 5000
    local phoneNum1 = 100
    local passwd = "yjz199595"
    local account1 = "%d@qq.com"
    local data1 = string.format(account1, count)
    local data2 = passwd .. tostring(count)
    local data3 = phoneNum1 + count
    local res = loginComponent.Register(data1,data2, data3)
end

local Update = function()
    while true do
        coroutine.start(function ()
            coroutine.start(CallChat)
            coroutine.start(CallMongo)
            coroutine.start(TestHttp)
        end)
        coroutine.sleep(100)
    end
end


function Main.Start()
    loginComponent.Register(account, password, phoneNum)

    local loginInfo = loginComponent.Login(account, password)
    if loginInfo == nil or loginInfo.code ~= XCode.Successful then
        Log.Error("使用http登陆失败")
        return false
    end

    table.print(loginInfo)
    local token = loginInfo.data.token
    local address = loginInfo.data.address
    if not Client.New(address, token) then
        Log.Error("连接网关服务器 [" , address, "] 失败")
        return false
    end
    Log.Debug("连接网关服务器[" , address, "]成功")
    coroutine.start(Update)
    return true
end


return Main