
local Main = {}
require("Player.Player")
require("component.ChatComponent")
require("component.LoginComponent")
local CallMongo = function()
    local t1 = Time.NowMilTime()
    local code, res = Client.Call("MongoDB.Query", {
        tab = "user.account_info",
        json = rapidjson.encode({
            _id = "646585122@qq.com"
        }),
        limit = 1
    })
    if code == XCode.Successful then
        table.print(res)
    end
    local t = Time.NowMilTime() - t1
    Console.Warn("cal MongoDB.Query [", t, "]ms");
end
local players = {}
function Main.Start()
    local accounts = require("Player.Account")
    for _, account in ipairs(accounts) do
        local player = Player.New(account.account, account.passwd, account.phoneNum)
        table.insert(players, player)
    end

    for _, player in ipairs(players) do
        player:Login()
    end
    for _, player in ipairs(players) do
        coroutine.start(player.Update, player)
    end
    return true
end
return Main