local log = require("Log")
local json = require("util.json")
local str_format = string.format
local timer = require("core.timer")
local Module = require("Module")
local Session = require("Session")
local http = require("HttpComponent")

local HOST = "http://43.143.239.75:80"
--local HOST = "http://127.0.0.1:8088"
local COUNT = os.getenv("APP_COUNT") or 100
local Main = Module()

local lastUserMemory = 0
local lastLuaUserMemory = 0
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
        local osInfo = os.get_system_info()
        local user_memory = osInfo.use_memory
        local luaMemory = collectgarbage("count")
        local t2 = (luaMemory - lastLuaUserMemory)
        local t1 = (user_memory - lastUserMemory) / 1024
        local t3 = user_memory / (1024 * 1024)
        log.Warning("sum:%.2fMB  add:%.2fKB  lua:%.2fKB", t3, t1, t2)

        lastUserMemory = user_memory
        lastLuaUserMemory = luaMemory
    end)

    for i = 1, COUNT do
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
                self:Login(player.account)
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
    for i = 1, 10 do
        client:Call("GateSystem.Ping")

        client:Call("ChatSystem.Ping")

        client:Call("ChatSystem.OnPing")

        client:Call("ChatSystem.OnChat", {
            msg_type = math.random(0, 3),
            message = "hello world"
        })
    end

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
    log.Debug("user(%s) start login", info.account)
    local response = http:Post(str_format("%s/account/login", HOST), info)
    if response == nil or response.data == nil then
        log.Error("account login failure")
        return
    end
    local result = response.data
    local client = Session(result.address)
    local timerId = timer.Add(2000, function()
        log.Error("user(%s) login [%s] time out", info.account, result.address)
    end)
    local code = client:Call("GateSystem.Login", result.token)
    print(code)
    timer.Del(timerId)
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