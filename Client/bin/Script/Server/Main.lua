Main = {}
require "ServiceProxy"
require "Coroutine"
function Main.Load()
    local hofix = require("HofixHelper")
end

function Main.Start()
    local address = SoEasy.NewService("AccountService")

    local registerData = {}
    registerData.account = "6465851222@qq.com"
    registerData.password = "199595yjz."
    registerData.phonenum = 13716061995
    registerData.platform = "ios_qq"
    registerData.device_mac = "0xs1dsx"

    print(address)
end
