
local log = require("Log")
local json = require("util.json")
local Component = require("Component")
local str_format = string.format

local LoginComponent = Component()
local http = require("HttpComponent")
local host = os.getenv("APP_HOST") or "http://127.0.0.1:80"

function LoginComponent:Update()

    self:Get(str_format("%s/admin/hello", host))
    self:Get(str_format("%s/admin/ping?id=%d", host, 1))
    self:Post(str_format("%s/admin/login", host), {
        account = "646585122@qq.com",
        passwd = "123456",
        dev_id = "iPone13 mini",
        dev = 'ios'
    })
end

function LoginComponent:Get(url)
    local t1 = time.ms()
    local response = http:Do("GET", url)
    log.Warning("(%s) time = [%s]ms response = %s", url, time.ms() - t1, json.encode(response))
end

function LoginComponent:Post(url, data)
    local t1 = os.clock() * 1000
    local response = http:Do("POST", url, nil, data)
    log.Warning("(%s) time = [%s]ms response = %s", url, os.clock() * 1000 - t1, json.encode(response))
end

function LoginComponent:Login(account, passwd) -- 获取gate地址
    local url = string.format("%s/%s", host, "account/login")
    local response = http:Post(url, {
        account = account,
        password = passwd
    })

    if response.code ~= XCode.Ok then
        log.Error("%s login code = %s", account, response.code)
        return
    end
    log.Debug("login response = %s", json.encode(response))
    return response
end
return LoginComponent