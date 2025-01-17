
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

    --local tcp = require("net.tcp")
    --local rsa = require("util.rsa")
    --local aes = require("util.aes")
    --local json = require("util.json")
    --local base64 = require("util.base64")
    --local client = rsa.create("./public.key", "private.key")
    --
    --local key = string.range(32)
    --
    --local input = json.encode({
    --    user_id = 10002,
    --    nick = "name",
    --    age = 10,
    --    list = { 101, 102, 103}
    --})
    --
    --local str = aes.encode(key, input)
    --print(key, string.len(input), base64.encode(str))
    --table.print(aes.decode(key, str))
end

return Main
