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
    for i = 1, 100 do
        local account = string.format("yjz1995%s", i)
        table.insert(self.accounts, { account = account, password = "123456 "})
    end
end

function Main:OnUpdate()
    for _, session in ipairs(self.sessions) do
        self.count = self.count + 1
        coroutine.start(self.CallServer, self, session)
    end
    print(string.format("======== [%d] =======", self.count))
end

function Main:CallServer(client)

    client:Call("ChatSystem.Ping")

    client:Call("ChatSystem.OnChat", {
        msg_type = 1,
        message = "chat on world"
    })

    client:Call("ChatSystem.Request")
    self.count = self.count - 1
end

function Main:OnComplete()

    for _, info in pairs(self.accounts) do
        coroutine.start(function()
            local response = http:Post(str_format("%s/account/login", HOST), info)
            if response == nil or response.data == nil then
                log.Error("account login failure")
                return
            end

            local result = response.data
            local client = Session(result.address)

            local code = client:Call("GateSystem.Login", result.token)
            if code == XCode.Ok then
                table.insert(self.sessions, client)
                log.Warning("user(%s) login [%s] ok", info.account, result.address)
            end
        end)
    end
end

return Main