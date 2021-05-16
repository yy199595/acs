
Main = {}
LoginManager = { }
local this = Main
require('Util.JsonUtil')
require('Action.Action')

function Main.Load()
    

    local hofix = require("Util.HofixHelper")
end

function LoginManager.Login(session, operId, messageData)
    SoEasy.Sleep(1000)
    print("----------------")
    return 1
end

function Main.Start()
    local table = { }
    table.name = "yuejianzheng"
    --SoEasy.BindAction("LoginManager.Login", LoginManager.Login)
    SoEasy.Start(function() 
       local code, json = MysqlClient.InvokeCommand("shouhuzhemen299_db", "select * from player_risk")
       print(code, JsonUtil.ToString(json))
    end)

   
end

