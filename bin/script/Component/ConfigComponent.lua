
local app = require("App")
local json = require("app.json")
local str_format = string.format
local ConfigComponent = app:Make()

function ConfigComponent:OnInit()
    self.rpcConfigs = { }
end

function ConfigComponent:GetConfig(name)
    return self.rpcConfigs[name]
end

function ConfigComponent:LoadConfig(path)
    local res = json.loadfile(path)
end

return ConfigComponent