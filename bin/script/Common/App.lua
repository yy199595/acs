

local App = _G.App or { }
local Actor = { }

local Server = { }
local Player = { }

function Player:Call(func, request)
    return App.Call(self.id, func, request)
end

function Player:Send()
    return App.Send(self.id, func, request)
end

function Player:SendToClient()
    return App.SendToClient(self.id, func, request)
end

function Server:Call()

end

function Server:Send()

end

function Actor.New(id, name)
    local tab = { }
    tab.actor_id = id
    tab.name = name
    tab.app = App
    setmetatable(tab, {__index = Actor})
    return tab
end

function Actor:Call(func, request)
    return self.app.Call(self.actor_id, func, request)
end

function Actor:Send(func, request)
    return self.app.Send(self.actor_id, func, request)
end


function App.NewActor(id, name)
    return Server.New(id, name)
end

function App.NewPlayer(id)
    return Player.New(id)
end

return App