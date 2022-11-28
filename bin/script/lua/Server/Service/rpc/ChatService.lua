
local ChatService = { }
local messageComponent = App.GetComponent("ProtoComponent")
local gateComponent = App.GetComponent("GateHelperComponent")

function ChatService.OnServiceStart()
    print("启动聊天服务")
end

ChatService.Chat = function(id, request)
    print("更新完成&&&&&&112233")
    local chatMessage = messageComponent:New("c2s.chat.notice", {
        msg_type = request.msg_type,
        message = request.message
    })

    gateComponent:BroadCast("ChatComponent.Chat", chatMessage)
    return XCode.Successful
end

ChatService.Test = function(id, request)
    --coroutine.sleep(0.5)
    Log.Error(id, Json.Encode(request))
    return XCode.Successful, request
end

return ChatService