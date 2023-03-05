
local Chat = { }
local gateComponent = App.GetComponent("GateHelperComponent")

function Chat.OnServiceStart()
    print("启动聊天服务")
end

Chat.Chat = function(id, request)
    print("更新完成&&&&&&112233", id)
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