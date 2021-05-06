
Main = { }
require "JsonUtil"


function Main.Load()
    local hofix = require('HofixHelper')
    hofix.LoadModuleByName("ServerManager")
    hofix.LoadModuleByName("NetWorkManager")
end

function Main.Start()
    local returnData = { }
    returnData.area_id = 299
    returnData.server_ip = "192.168.0.183"
    returnData.server_port = 1995
end

function Main.Update(delatime)
   
end