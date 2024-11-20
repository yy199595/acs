local log = require("Log")
local json = require("util.json")
local str_format = string.format
local timer = require("core.timer")
local Module = require("Module")
local Session = require("Session")
local http = require("HttpComponent")

local HOST = "http://127.0.0.1:8088"

local Main = Module()

function Main:Awake()
    self.count = 0
    self.accounts = { }
    self.sessions = { }
    timer.AddUpdate(500, self, "OnUpdate")
    timer.AddUpdate(2000, function()
        print(string.format("coroutine count = %s", self.count))
    end)

    for i = 1, 100 do
        local account = string.format("yjz1995%s", i)
        table.insert(self.accounts, { account = account, password = "123456" })
    end
end

function Main:OnUpdate()
    for index, player in ipairs(self.sessions) do
        if player.close then
            table.remove(self.sessions, index)
            break
        end
        self.count = self.count + 1
        coroutine.start(self.CallServer, self, player)
    end
end

function Main:CallServer(player)

    local client = player.client
    client:Call("GateSystem.Ping")

    client:Call("ChatSystem.Ping")

    client:Call("ChatSystem.OnChat", {
        msg_type = 1,
        message = "chat on world"
    })

    client:Call("ChatSystem.Request")

    player.count = player.count + 1

    if player.count >= 10 then
        local code = client:Call("GateSystem.Logout")

        client:Close()
        player.close = true
        print(string.format("user(%s) logout ok", player.account.account))

    end

    self.count = self.count - 1
end

function Main:OnComplete()

    for i, info in pairs(self.accounts) do
        coroutine.start(function()
            coroutine.sleep(i * 20)
            local response = http:Post(str_format("%s/account/login", HOST), info)
            if response == nil or response.data == nil then
                log.Error("account login failure")
                return
            end

            local result = response.data
            local client = Session(result.address)

            local code = client:Call("GateSystem.Login", result.token)
            if code == XCode.Ok then
                table.insert(self.sessions, {
                    index = i,
                    count = 0,
                    close = false,
                    account = info,
                    client = client,
                })
                log.Warning("[%s] user(%s) login [%s] ok", i, info.account, result.address)
            end
        end)
    end
end

return Main