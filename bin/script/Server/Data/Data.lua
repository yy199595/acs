
local Module = require("Module")

local Main = Module()
local log = require("Log")
local mongo = require("MongoComponent")
function Main:Awake()

end

function Main:OnComplete()
    log.Info("start init mongo index")
    mongo:SetIndex("user.account", { "user_id"})
    mongo:SetIndex("player.account", { "user_id"})
end

return Main