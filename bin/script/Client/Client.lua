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
    self.login_count = 0
    self.accounts = { }
    self.sessions = { }
    timer.AddUpdate(500, self, "OnUpdate")
    timer.AddUpdate(2000, function()
        local count = 0
        for _, player in ipairs(self.sessions) do
            count = count + player.count
        end
        print(string.format("coroutine count(%s) rpc_count[%s]", self.count, count))
    end)

    for i = 1, 100 do
        local account = string.format("yjz1995%s", i)
        table.insert(self.accounts, { account = account, password = "123456" })
    end
end

function Main:OnUpdate()
    for index, player in ipairs(self.sessions) do
        if player.close then
            coroutine.start(function()
                local sleep = math.random(100, 2000)
                coroutine.sleep(sleep)
                self:Login(player)
            end)
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

    if player.count >= player.logout_count then
        local code = client:Call("GateSystem.Logout")

        client:Close()
        player.close = true
        print(string.format("user(%s) logout ok", player.account.account))

    end

    self.count = self.count - 1
end

function Main:Login(info)
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
            count = 0,
            close = false,
            account = info,
            client = client,
            logout_count = math.random(100, 10000)
        })
        self.login_count = self.login_count + 1
        log.Warning("[%s] user(%s) login [%s] ok", self.login_count, info.account, result.address)
    end
end

function Main:OnComplete()

    for _, info in pairs(self.accounts) do
        coroutine.start(self.Login, self, info)
    end
end

return Main