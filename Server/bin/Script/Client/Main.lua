require "TxtUtil"
require "JsonUtil"
require "ClientManager"

Main = {}

function Main.Load()

end

function Main.Start()

    local registerData = {}
    registerData.account = "6465851222@qq.com"
    registerData.password = "199595yjz."
    registerData.phonenum = 13716061995
    registerData.platform = "ios_qq"
    registerData.device_mac = "0xs1dsx"

    local count = 0
    SoEasy.AddTimer(
            function()
                count = count + 1
                local registerInfo = SoEasy.CreateByTable("PB.UserRegisterData", registerData)
                SoEasy.Warning(registerInfo, count)
            end, 1000, 10)
end

