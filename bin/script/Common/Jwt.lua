
local app = require("App")
local ljwt = require("auth.jwt")

local jwt = { }
local core = app:GetConfig("core")

function jwt.Create(data)
    local key = core.secret
    return ljwt.Create(key, data)
end

function jwt.Verify(token)
    local key = core.secret
    return ljwt.Verify(token, key)
end

return jwt