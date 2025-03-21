
local client = require("net.client")

local Session = {
    close = false
}
function Session:Send(func, message)
    if self.close then
        return XCode.SendMessageFail
    end
    self.count = self.count + 1
    return client.Send(self.id, func, message)
end

function Session:Call(func, message)
    if self.close then
        return XCode.SendMessageFail
    end
    self.count = self.count + 1
    return client.Call(self.id, func, message)
end

function Session:Close()
    self.close = true
    return client.Close(self.id)
end

return function(address)
    local session = Class(Session)
    session.count = 0
    session.id = client.Connect(address)
    return session
end