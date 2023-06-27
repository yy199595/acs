
require("XCode")
local app = require("App")

local Component = { }

function Component:OnInit()
    self.app = app
end

function Component:GetName()
    return self.name
end

function Component:Send(id, func, request)
    return self.app.Send(id, func, request)
end

function Component:Call(id, func, request)
    return self.app.Call(id, func, request)
end

function Component:GetListen(name)
    return self.app.GetListen(name)
end

function Component:Allot(name)
    return self.app.Random(name)
end

return Component