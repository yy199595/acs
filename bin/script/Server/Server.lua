local proto = require("Proto")
local Module = require("Module")
local http = require("HttpComponent")
local mongo = require("MongoComponent")
local redis = require("RedisComponent")

local Main = Module()

SetMember(Main, "count", 1)

proto.Import("record/record.proto")

function Main:Awake()

end

function Main:OnComplete()

    local appId = 2
    local app = require("App")
    local log = require("Log")
    print(app:AddListen(appId, "rpc", "43.143.239.75:7789"))
    print(app:AddListen(appId, "kcp", "43.143.239.75:7787"))

    local code = app:Call(appId, "ChatSystem.Request", {

    })
    print(code)
    for i = 1, 10 do
        coroutine.sleep(200)
        local t1 = os.clock()
        code, _ = app:Call(appId, "ChatSystem.OnChat", {
            user_id = 10004,
            msg_type = 1,
            message = "hello"
        })
        local t2 = os.clock()

        log.Warning("(%s)ms [%s]  code:%s", t2 - t1, i, code)
    end


    --local res = http:Get("https://huwai.pro")
    --table.print(res)
    --local oss = require("util.oss")
    --local url = oss.Upload("C:/Users/64658/Desktop/yy/ace/bin/www/dist/bg.jpg", "10000")
    --print(url)
end

return Main
