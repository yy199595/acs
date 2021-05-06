
Main = {}
local this = Main
require('JsonUtil')
print("start script")
function Main.Load()
    local hofix = require("HofixHelper")
    hofix.LoadModuleByName("ServerManager")
    hofix.LoadModuleByName("NetWorkManager")
end

function Main.Start()
   
end

