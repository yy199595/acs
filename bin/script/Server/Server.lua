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

    local app = require("App")
    local log = require("Log")
    print(app:AddListen(nil, "kcp", "127.0.0.1:10004"))
    for i = 1, 10 do
        coroutine.sleep(200)
        local code, _ = app:Call(nil, "ChatSystem.OnChat", {
            user_id = 10004,
            msg_type = 1,
            message = "hello"
        })
        log.Warning("[%s]  code:%s", i, code)
    end


    --local res = http:Get("https://huwai.pro")
    --table.print(res)
    --local oss = require("util.oss")
	--local url = oss.Upload("C:/Users/64658/Desktop/yy/ace/bin/www/dist/bg.jpg", "10000")
	--print(url)
end

return Main
