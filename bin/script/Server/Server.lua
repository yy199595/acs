local proto = require("Proto")
local Module = require("Module")
local http = require("HttpComponent")
local mongo = require("MongoComponent")
local redis = require("RedisComponent")

local Main = Module()
local log = require("Log")

SetMember(Main, "count", 1)

proto.Import("record/record.proto")

local app = require("App")
function Main:Awake()

    local appId = 1

    app:MakeServer(appId, "server", {
        rpc = "43.143.239.75:7789",
        kcp = "43.143.239.75:7787",
        udp = "43.143.239.75:7788"
    })
    local timer = require("core.timer")
    timer.AddUpdate(300, self, "Update")
end

local count = 0
function Main:Update()
    print(string.format("==========================%s=========================", count))
    coroutine.start(function()
        count = count + 1
        for i = 1, 2 do
            coroutine.sleep(100)
            local t1 = os.clock()
            local code, _ = app:Call(appId, "ChatSystem.OnChat", {
                user_id = 10004,
                msg_type = 1,
                message = "hello"
            })
            local t2 = os.clock()
            log.Info("(kcp)  (%s)ms [%s]  code:%s", t2 - t1, i, code)
            code = app:Call(appId, "ChatSystem.Request", {
                name = "xiao",
                age = 10 + i,
                index = i
            })
            local t3 = os.clock()
            log.Warning("(udp)  (%s)ms [%s]  code:%s", t3 - t2, i, code)
            code = app:Call(appId, "ChatSystem.Ping")
            local t4 = os.clock()
            log.Error("(tcp)  (%s)ms [%s]  code:%s", t4 - t3, i, code)
        end
        count = count - 1
    end)
end

function Main:OnComplete()





    --local res = http:Get("https://huwai.pro")
    --table.print(res)
    --local oss = require("util.oss")
    --local url = oss.Upload("C:/Users/64658/Desktop/yy/ace/bin/www/dist/bg.jpg", "10000")
    --print(url)
end

return Main
