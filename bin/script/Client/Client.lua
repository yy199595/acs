
require("XCode")
local random = math.random
local log = require("Log")
local str_format = string.format
local timer = require("core.timer")
local Module = require("Module")
local Session = require("Session")
local http = require("HttpComponent")

local tab_insert = table.insert
local coroutine_sleep = coroutine.sleep
local coroutine_create = coroutine.create
local coroutine_resume = coroutine.resume
--local HOST = "http://43.143.239.75:80"
local HOST = "http://127.0.0.1:8088"
local COUNT = os.getenv("APP_COUNT") or 1
local Main = Module()

local rpc_count = 0
local http_count = 0

function Main:Awake()

    self.count = 0
    self.login_count = 0
    self.accounts = { }
    self.sessions = { }
    for i = 1, COUNT do
        local account = string.format("yjz1995%s", i)
        tab_insert(self.accounts, { account = account, password = "123456", count = 0 })
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
        http_count = http_count + 1
        local result = response.data
        local client = Session(result.address)
        local code = client:Call("GateSystem.Login", result.token)
        if code == XCode.Ok then
            info.client = client
            info.count = info.count + 1
            info.max_count = random(100, 500)

            tab_insert(self.sessions, info)
            self.login_count = self.login_count + 1
            log.Info("[%s] user(%s) login [%s] ok rpc:%s http:%s",
                    self.login_count, info.account, result.address, rpc_count, http_count)
        else
            log.Error("user(%s) login [%s] fail", info.account, result.address)
            return
        end
    end
    http:Get(str_format("%s/admin/info", HOST))
    info.client:Call("GateSystem.Ping")
    info.client:Call("ChatSystem.Ping")
    info.client:Call("ChatSystem.OnPing")
    info.client:Call("ChatSystem.OnChat", {
        msg_type = math.random(0, 3),
        message = "hello world"
    })
    rpc_count = rpc_count + 4
    info.count = info.count + 4
    if info.count >= info.max_count then
        info.client:Close()
        info.count = 0
    end
    coroutine_sleep(200)
    local co = coroutine_create(self.Login)
    coroutine_resume(co, self, info)
end

function Main:OnComplete()

    for i, info in ipairs(self.accounts) do
        timer.Add(100 * i, function()
            local co = coroutine_create(self.Login)
            coroutine_resume(co, self, info)
        end)
    end
end

return Main