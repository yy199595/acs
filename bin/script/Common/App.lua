

App = _G.App or { }
local this = App
local Service = require("Service")

function App.NewRpcService(name)
    if this.rpcServices == nil then
        this.rpcServices = { }
    end
    local rpcService = this.rpcServices[name]
    if rpcService == nil then
        rpcService = Service.New(name)
        this.rpcServices[name] = rpcService
    end
    return rpcService
end

return App