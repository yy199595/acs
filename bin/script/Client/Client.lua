local log = require("Log")
local json = require("util.json")
local str_format = string.format
local timer = require("core.timer")
local Module = require("Module")
local client = require("net.client")
local http = require("HttpComponent")

local HOST = "http://127.0.0.1:8088"

local Main = Module()

function Main:Awake()
    local str = require("util.str")

    local t1 = os.clock()
    for i = 1, 100000 do
        str.format("bool={} int={} str={}", true, 10, "1011")
    end
    local t2 = os.clock()
    for i = 1, 100000 do
        string.format("bool=%s int=%s str=%s", true, 10, "1011")
    end
    local t3 = os.clock()

    print(string.format("====%s  ----------%s", t2 - t1, t3 - t2))
end

function Main:OnUpdate()
    print("=============")
end

function Main:OnComplete()
    local response = http:Post(str_format("%s/account/login", HOST), {
        account = "yjz1995",
        password = "199595yjz."
    })
    if response == nil or response.data == nil then
        log.Error("account login failure")
        return
    end
    local result = response.data
    local fd = client.Connect(result.address)
    client.Call(fd, "GateSystem.Login", result.token)

    client.Call(fd, "ChatSystem.OnChat", {
        msg_type = 1,
        message = "chat on world"
    })

    client.Call(fd, "ChatSystem.OnChat", {
        user_id = 1,
        msg_type = 2,
        message = "chat on private"
    })

    coroutine.sleep(1000)
    --client.Call(fd, "GateSystem.Logout")
end

return Main