Main = {}
local this = Main
require"ServiceProxy"

function Main.Load()
    local hofix = require('HofixHelper')
end

function Main.Start()
   local address = SoEasy.NewService("LoginService")

   local registerData = { }
   registerData.account = "6465851222@qq.com"
   registerData.password = "199595yjz."
   registerData.phonenum = 13716061995
   registerData.platform = "ios_qq"
   registerData.device_mac = "0xs1dsx"

   local message = SoEasy.CreateByTable("PB.UserRegisterData", registerData)


   print(address)
end
