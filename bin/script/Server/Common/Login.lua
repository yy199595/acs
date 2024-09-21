
local app = require("App")
local log = require("Log")
local name = os.getenv("name")

local RpcService = require("RpcService")

local Login = RpcService()

function Login:_OnLogin(playerId)
    app:Send(playerId, "ChatComponent.OnChat", {
        user_id = playerId,
        msg_type = 1,
        message = "hello"
    })
    log.Warning("player (%s) login %s successful", playerId, name)
end

function Login:_OnLogout(playerId)
    log.Warning("player (%s) logout %s successful", playerId, name)
end

return Login