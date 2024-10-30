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
    for i = 1, 10 do
        coroutine.sleep(200)
        local code, res = app:Call(nil, "ChatSystem.OnChat", {
            user_id = 10004,
            msg_type = 1,
            message = "hello"
        })
        log.Warning("[%s]  code:%s", i, code)
    end

    coroutine.start(function()
        local kcp = KcpSocket.New("127.0.0.1:7787")
        for i = 1, 10 do
            coroutine.sleep(200)
            kcp:Send("hello world")
        end
    end)


    --local res = http:Get("https://huwai.pro")
    --table.print(res)
    --local oss = require("util.oss")
	--local url = oss.Upload("C:/Users/64658/Desktop/yy/ace/bin/www/dist/bg.jpg", "10000")
	--print(url)
end

return Main
