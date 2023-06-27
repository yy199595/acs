
local Chat = Class("RpcService")

function Chat:Awake()
    self.app:AddWatch("Gate")
end

function Chat:OnLogin(userId)
    coroutine.sleep(1000)
    print(string.format("玩家%d登录聊天服务", userId))
end

function Chat:Chat(request)
    local player_id = request.id
    local message = request.message
    self.app:SendToClient(player_id, "ChatComponent.Chat", {
        msg_type = message.msg_type,
        message = message.message
    })
    return XCode.Successful
end
local count = 0

function Chat:Ping(request)
    count = count + 1
    print(os.time(), "count = " .. count)
    return XCode.Successful
end

return Chat