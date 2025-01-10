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
    --local timer = require("core.timer")
    --timer.AddUpdate(400, self, "Update")
end

local count = 0
function Main:Update()
    --print(string.format("==========================(%s)=========================", count))
    for i = 1, 20 do
        coroutine.start(function()
            count = count + 1
            local code = app:Call(appId, "ChatSystem.OnChat", {
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
            code = app:Call(appId, "GateSystem.Ping")
            mongo:FindOne("demo.player", { _id = 10001 })
            --http:Get("http://127.0.0.1:80/admin/hello")
            --http:Get("http://127.0.0.1:80/admin/all_info")
            --http:Get("http://127.0.0.1:80/admin/ping?id=0")

            count = count - 1
        end)
    end

end

function Main:OnComplete()

    local bson = require("util.bson")
    local json = require("util.json")

    local request = {
        func = "ChatSystem.OnChat",
        player_id = 10000999,
        name = "xiaoming",
        friend_list = {
            {
                id = 1001,
                name = "xiaozhang"
            },
            {
                id = 1002,
                name = "xiaohua"
            }
        },
        sex = true,
        list = { 1, 2, 3, 44, 5 }
    }

    local sum = 100000
    local str1 = bson.encode(request)
    local str2 = json.encode(request)
    local t1 = os.clock()
    for i = 1, sum do
        --local str = bson.encode(request)
       bson.decode(str1)
    end
    local t2 = os.clock()
    for i = 1, sum do
        --local str = json.encode(request)
        json.decode(str2)
    end
    local t3 = os.clock()
    print(string.format("[%s]bson=%s  [%s]json=%s", #str1, t2- t1, #str2, t3 - t2))
end

return Main
