
local log = require("Log")
local json = require("util.json")
local console = require("Console")
local router = require("net.client")
local RpcService = require("RpcService")
local ChatComponent = RpcService()

function ChatComponent:Awake()
    self.is_connected = false
end

function ChatComponent:OnChat(request)
    console.Warning("server call client = %s", json.encode(request))
    return XCode.Ok
end

function ChatComponent:Update(fd)
    self:CallChat(fd)
    self:CallChat(fd)
end

function ChatComponent:CallPing(fd)
    local t1 = os.clock() * 1000
    router.Call(fd, "ChatSystem.Ping")
    --log.Debug("ping chat time = [%s]ms", os.clock() * 1000 - t1)
end

function ChatComponent:CallChat(fd)

    local t1 = os.clock() * 1000
    local code = router.Call(fd, "ChatSystem.OnChat", {
        msg_type = 1,
        message = "hello"
    })
    --log.Debug("chat.chat = [%s]ms code = %s", os.clock() * 1000 - t1, code)
end

return ChatComponent