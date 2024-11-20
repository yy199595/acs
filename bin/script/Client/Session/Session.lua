
local client = require("net.client")

local Session = { }
function Session:Send(func, message)
    return client.Send(self.id, func, message)
end

function Session:Call(func, message)
    return client.Call(self.id, func, message)
end

return function(address)
    local session = Class(Session)
    session.id = client.Connect(address)
    return session
end