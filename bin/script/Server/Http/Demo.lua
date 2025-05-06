
local log = require("Log")
local HttpService = require("HttpService")

local Demo = HttpService()

function Demo:Awake()
    self.count = 0
end

function Demo:Ping(request)
    self.count = self.count + 1
    log.Debug("count => {}", self.count)
    return XCode.Ok, "Pong"
end

function Demo:Regex(request)
    table.print(request)
    return XCode.Ok
end

return Demo