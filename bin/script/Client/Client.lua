require("XCode")
require("Coroutine")
local random = math.random
local log = require("Log")
local str_format = string.format
local timer = require("core.timer")
local Module = require("Module")
local Session = require("Common.Session")
local http = require("HttpComponent")

local tab_insert = table.insert
local coroutine_sleep = coroutine.sleep
local coroutine_create = coroutine.create
local coroutine_resume = coroutine.resume

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

function Main:OnStart()

end

local info = {
    user_id = 4,
    create_time = 8,
    player_name = "",
    level = 4,
    login_time = 8
}

local count = 0
local host = "http://127.0.0.1:8088"
--local host = "http://43.143.239.75:8080"

local start_redis = function()
    local url = str_format("%s/db/redis", host)
    while true do
        count = count + 1
        http:Post(url, { "SET", "now_time", os.time() })
        http:Post(url, { "GET", "now_time" })
        http:Post(url, { "INFO"})
    end
end

local start_ping = function()
    while true do
        count = count + 1
        http:Get(str_format("%s/admin/ping", host))
    end
end

local start_hello = function()
    while true do
        count = count + 1
        http:Get(str_format("%s/hello", host))
    end
end

local start_mysql = function()
    local tab = "demo"
    local sqlHelper = require("SqlHelper")
    local url = str_format("%s/db/mysql", host)
    while true do
        count = count + 1

        local res2 = http:Post(url, sqlHelper.CreateSql(tab, info))
        local res3 = http:Post(url, sqlHelper.InsertSql(tab, info))
        local res4 = http:Post(url, sqlHelper.QuerySql(tab, { user_id = info.user_id }))
        local res5 = http:Post(url, sqlHelper.DeleteSql(tab, { user_id = info.user_id }))

    end
end

local start_mongo = function()
    local tab = "demo"
    local url = str_format("%s/db/mongo", host)
    while true do
        count = count + 1

        http:Post(url, { cmd = "InsertOnce", documents = { tab, info } })
        http:Post(url, { cmd = "FindOne", documents = { tab, { user_id = info.user_id } } })
        http:Post(url, { cmd = "Delete", documents = { tab, { user_id = info.user_id }, 1 } })

    end
end

local start = 0

local start_run_info = function()
    local url = str_format("%s/admin/info", host)
    while true do
        count = count + 1
        local response = http:Get(url).data
        local value = response.const_memory_b - start
        if value ~= 0 then
            print("value = ", value)
            start = response.const_memory_b
        end
    end
end

function Main:OnComplete()
    for i = 1, 5 do
        coroutine.start(start_ping)
        coroutine.start(start_hello)

        coroutine.start(start_redis)
        coroutine.start(start_mysql)
        coroutine.start(start_mongo)
        coroutine.start(start_run_info)
    end
end

function Main:OnUpdate(tick)
    if tick % 5 == 0 then
        log.Debug("count = {}", count)
    end
end

return Main