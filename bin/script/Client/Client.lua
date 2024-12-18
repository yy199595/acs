local log = require("Log")
local json = require("util.json")
local str_format = string.format
local timer = require("core.timer")
local Module = require("Module")
local Session = require("Session")
local http = require("HttpComponent")

--local HOST = "http://43.143.239.75:80"
local HOST = "http://127.0.0.1:8088"
local COUNT = os.getenv("APP_COUNT") or 1
local Main = Module()

local lastUserMemory = 0
local lastLuaUserMemory = 0
function Main:Awake()


    self.count = 0
    self.login_count = 0
    self.accounts = { }
    self.sessions = { }
    timer.AddUpdate(200, self, "OnUpdate")
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
        --log.Warning("sum:%.2fMB  add:%.2fKB  lua:%.2fKB", t3, t1, t2)

        lastUserMemory = user_memory
        lastLuaUserMemory = luaMemory
    end)

    for i = 1, COUNT do
        local account = string.format("yjz1995%s", i)
        table.insert(self.accounts, { account = account, password = "123456", count = 0 })
    end
end

function Main:OnUpdate()
    for i = 1, 10 do
        for _, info in pairs(self.accounts) do
            coroutine.start(self.Login, self, info)
        end
    end
end

function Main:Login(info)
    if info.count == 0 then
        --log.Debug("user(%s) start login", info.account)
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
            info.client = client
            info.count = info.count + 1
            table.insert(self.sessions, info)
            self.login_count = self.login_count + 1
            log.Warning("[%s] user(%s) login [%s] ok", self.login_count, info.account, result.address)
        end
    end
    if info.count > 0 then
        for i = 1, 10 do
            local code1 = info.client:Call("GateSystem.Ping")
            local code2 = info.client:Call("ChatSystem.Ping")
            local code3 = info.client:Call("ChatSystem.OnPing")
            local code4 = info.client:Call("ChatSystem.OnChat", {
                msg_type = math.random(0, 3),
                message = "hello world"
            })
            info.count = info.count + 4
            print(code1, code2, code3, code4)
            log.Info("user(%s) call count:%s", info.account, info.count)
        end
        info.client:Call("GateSystem.Logout")

        info.client:Close()
        info.count = 0
    end

end

function Main:OnComplete()

    for _, info in pairs(self.accounts) do
        coroutine.start(self.Login, self, info)
    end
end

return Main