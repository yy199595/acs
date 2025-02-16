
local app = require("App")
local ljwt = require("util.jwt")

local jwt = { }
local core = app:GetConfig("core")

function jwt.Create(data)
    local key = core.secret
    return ljwt.encode(key, data)
end

function jwt.Verify(token)
    local key = core.secret
    return ljwt.decode(token, key)
end

return jwt