
local Agent = { }
local fmt = string.format
function Agent.New(app, id, name)
    local agent = {
        id = id,
        app = app,
        name = name
    }

    setmetatable(agent, {
        __index = Agent,
        __tostring = function(self)
            return fmt("[%s]:(%s)", self.name, self.id)
        end
    })
    return agent
end

function Agent:Call(method, request)
    local func = fmt("%s.%s", self.name, method)
    return self.app:Call(self.id, func, request)
end

function Agent:Send(method, request)
    local func = fmt("%s.%s", self.name, method)
    return self.app:Send(self.id, func, request)
end

function Agent:SendToClient(method, request)
    return self.app:SendToClient(method, request)
end

return Agent