Main = {}
local this = Main
require('Util.JsonUtil')
require('Action.Action')

function Main.Load()
    local hofix = require('Util.HofixHelper')
end

function Main.Start()
   local address = SoEasy.NewService("LoginService")
   print(address)
end
