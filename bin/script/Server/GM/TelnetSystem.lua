
local app = require("App")
local node = require("Node")
local timer = require("core.timer")
local json = require("util.json")
local Module = require("RpcService")

local TelnetSystem = Module()

local Command = { }

function Command.Stop()
    app:Stop()
    return XCode.Ok
end

function Command.Refresh()
    return node:Call(nil, "NodeSystem.Hotfix")
end

function Command.Http(args)
    local http = require("HttpComponent")
    local method, url, head, body = table.unpack(args)
    return XCode.Ok, http:Do(method, url, head and json.decode(head), body and json.decode(body))
end

function Command.Backup(args)
    local jwt = require("Jwt")
    local name = table.unpack(args)
    local http = require("HttpComponent")
    local address = node:GetListen(nil, "http")
    local url = string.format("http://%s/mongo/backup?name=%s", address, name)
    local response = http:Do("GET", url, { Authorization = jwt.Create({ u = 1, p = 100 }) })
    return XCode.Ok, response
end

function Command.Redis(args)
    if args == nil or not next(args) then
        return XCode.CallArgsError
    end
    local redis = require("RedisComponent")
    return XCode.Ok, redis:Run(table.unpack(args))
end

function Command.Mysql(args)
    if args == nil or not next(args) then
        return XCode.CallArgsError
    end
    local mysql = require("db.mysql")
    local sql = table.concat(args, ' ')
    return XCode.Ok, mysql.run(sql)
end

function Command.Info()
    local code, data = node:Call(nil, "NodeSystem.RunInfo")
    local response = json.decode(data.json)
    local result = { }
    for key, info in pairs(response) do
        local value = info
        if type(info) == "table" then
            value = json.encode(info)
        end
        table.insert(result, string.format("%s = %s", key, value))
    end
    return code, table.concat(result, "\r\n")
end

function TelnetSystem:AddAction(cmd, func, desc)
    self.commands[cmd] = {
        action = func,
        desc = desc
    }
end


function TelnetSystem:OnAwake()
    self.commands = { }
    self:AddAction("help", Command.Help, "look cmd")
    self:AddAction("stop", Command.Stop, "stop server")
    self:AddAction("info", Command.Info, "get server run info")
    self:AddAction("redis", Command.Redis, "run redis command")
    self:AddAction("mysql", Command.Mysql, "run mysql command")
    self:AddAction("http", Command.Http, "request by http")
    self:AddAction("backup", Command.Backup, "backup mongodb data")
    self:AddAction("refresh", Command.Hotfix, "invoke IRefresh")
end

function TelnetSystem:Run(request)
    local cmd = request.data.cmd
    local info = self.commands[cmd]
    if info == nil then
        return XCode.CallFunctionNotExist
    end
    if cmd == "help" then
        local help = { }
        for key, item in pairs(self.commands) do
            table.insert(help, string.format("%s => %s", key, item.desc))
        end
        return XCode.Ok, table.concat(help, "\r\n")
    end
    return info.action(request.data.args)
end

return TelnetSystem