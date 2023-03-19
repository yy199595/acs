
local Chat = { }

function Chat.Start()
    print("启动聊天服务")
    return true
end

function Chat.OnLogin(userId)
    coroutine.sleep(1000)
    print(string.format("玩家%d登录聊天服务",userId))
end

function Chat.Chat(id, request)
    local chatMessage = Proto.New("c2s.chat.notice", {
        msg_type = request.msg_type,
        message = request.message
    })
    Gate.Send(id, "ChatComponent.Private", chatMessage)
    Gate.BroadCast("ChatComponent.Chat", chatMessage)
    return XCode.Successful
end
local count = 0

function Chat.Ping(id)
    count = count + 1
    print(os.time(), "count = " .. count)
    return XCode.Successful
end

return Chat