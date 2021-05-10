
Main = {}
LoginManager = { }
local this = Main
require('Util.JsonUtil')
require('CoroutineManager.CoroutineAction')

function Main.Load()
    local hofix = require("Util.HofixHelper")
end

function LoginManager.Login(session, operId, messageData)

end

function Main.Start()
    local table = { }
    table.name = "yuejianzheng"
    SoEasy.BindAction("LoginManager.Login", LoginManager.Login)
    SoEasy.SendByAddress("127.0.0.1:7788", 1004654, 0, JsonUtil.ToString())
end

