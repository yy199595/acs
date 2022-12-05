
Client = {}

function Client.Test(tab)
    print(Json.Encode(tab))
end

local account = "yjz0995"
local password = "123456"
local phoneNum = 13716061995
local clientComponent = App.GetComponent("ClientComponent")
local loginComponent = require("component.LoginComponent")

local sumTime = 0
local record = {
    mongo = 0,
    chat = 0,
    ping = 0,
    register = 0
}
local CallMongo = function()
    
        record.mongo = record.mongo + 1
        local t1 = Time.NowMilTime()
        local code = clientComponent:Call("MongoService.Query", {
            tab = "user.account",
            json = Json.Encode({
                _id = "646585122@qq.com"
            }),
            limit = 1
        })
        record.mongo = record.mongo - 1;
        sumTime = sumTime + (Time.NowMilTime() - t1)
end

local CallChat = function()
    
        record.chat = record.chat + 1
        local t1 = Time.NowMilTime()
        local code = clientComponent:Call("ChatService.Ping", {
            user_id = 1122, msg_type = 1, message = "hello"
        })
        if code == XCode.Successful then
            
        end
        record.chat = record.chat - 1
        sumTime = sumTime + (Time.NowMilTime() - t1)
end

local TestHttp = function()
    local count = 5000
    local phoneNum = 100
    local passwd = "yjz199595"
    local account = "%d@qq.com"
    record.register = record.register + 1
        local data1 = string.format(account, count)
        local data2 = passwd .. tostring(count)
        local data3 = phoneNum + count
        local res = loginComponent.Register(data1,data2, data3)
        record.register = record.register  - 1
end


function Client.Start()

    loginComponent.Register(account, password, phoneNum)

    local loginInfo = loginComponent.Login(account, password)
    if loginInfo == nil or loginInfo.code ~= XCode.Successful then
        Log.Error("使用http登陆失败")
        return false
    end

    table.print(loginInfo)
    local address = loginInfo.data.address
    if not clientComponent:StartConnectAsync(address) then
        Log.Error("连接网关服务器 [" , address, "] 失败")
        return false
    end
    Log.Debug("连接网关服务器[" , address, "]成功")

    local code, _ = clientComponent:Auth(loginInfo.data.token)
    if code ~= XCode.Successful then
        Log.Error("user auth failure")
        return false
    end

    coroutine.start(function ()
        for i = 1, 100 do
            coroutine.start(CallChat)   
        end
        coroutine.sleep(1000)
        for i = 1, 100 do
            coroutine.start(CallMongo)      
        end
        coroutine.sleep(1000)
        -- for i = 1, 100 do
        --     coroutine.start(TestHttp)     
        -- end
    end)

    return true
end

Client.Update = function(tick)
    table.print(record)
end

return Client