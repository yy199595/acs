
local Chat = { }
local gateComponent = App.GetComponent("GateHelperComponent")

function Chat.OnServiceStart()
    print("启动聊天服务")
end

Chat.OnLogin = function(userId)
    print(string.format("玩家%d登录聊天服务",userId))
end

Chat.Chat = function(id, request)
    local chatMessage = Proto.New("c2s.chat.notice", {
        msg_type = request.msg_type,
        message = request.message
    })

    gateComponent:BroadCast("ChatComponent.Chat", chatMessage)
    return XCode.Successful
end
local count = 0

Chat.Ping = function(id)
    count = count + 1
    print(os.time(), "count = " .. count)
    return XCode.Successful
end

return Chat