
local Chat = { }
table.print(gate)
require("Component.MongoComponent")
function Chat.Start()
    print("启动聊天服务")
    return true
end

Chat.OnLogin = function(userId)
    coroutine.sleep(10000)
    print(string.format("玩家%d登录聊天服务",userId))
end

Chat.Chat = function(id, request)
    local chatMessage = Proto.New("c2s.chat.notice", {
        msg_type = request.msg_type,
        message = request.message
    })
    Gate.Send(id, "ChatComponent.Private", chatMessage)
    Gate.BroadCast("ChatComponent.Chat", chatMessage)
    return XCode.Successful
end
local count = 0

Chat.Ping = function(id)
    count = count + 1
    print(os.time(), "count = " .. count)
    return XCode.Successful
end

return Chat