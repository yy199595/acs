local proto = require("Proto")
local Module = require("Module")
local http = require("HttpComponent")
local mongo = require("MongoComponent")
local redis = require("RedisComponent")

local Main = Module()
local log = require("Log")

SetMember(Main, "count", 1)

proto.Import("record/record.proto")

local appId = nil
local app = require("App")
function Main:Awake()


    --app:MakeServer(appId, "server", {
    --    rpc = "43.143.239.75:7789",
    --    http = "43.143.239.75:80"
    --})
    local timer = require("core.timer")
    timer.AddUpdate(400, self, "Update")
end

local count = 0
function Main:Update()
    --print(string.format("==========================(%s)=========================", count))
    for i = 1, 10 do
        coroutine.start(function()
            count = count + 1
            for i = 1, 2 do
                local code, _ = app:Call(appId, "ChatSystem.OnChat", {
                    user_id = 10004,
                    msg_type = 1,
                    message = "hello"
                })
                code = app:Call(appId, "ChatSystem.Request", {
                    name = "xiao",
                    age = 10 + i,
                    index = i
                })
                code = app:Call(appId, "ChatSystem.Ping")
            end
            count = count - 1
        end)
    end

end

function Main:OnComplete()





    --local res = http:Get("https://huwai.pro")
    --table.print(res)
    --local oss = require("util.oss")
    --local url = oss.Upload("C:/Users/64658/Desktop/yy/ace/bin/www/dist/bg.jpg", "10000")
    --print(url)
end

return Main
