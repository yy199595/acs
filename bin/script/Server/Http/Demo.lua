
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

return Demo