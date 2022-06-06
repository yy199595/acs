
ChatService = {}
function ChatService.Awake()

    return true
end

ChatService.Chat = function(id, request, response)
    coroutine.sleep(2.5)
    local messageComponent = App.GetComponent("MessageComponent")
    local gateComponent = App.GetComponent("GateProxyComponent")
    local chatMessage = messageComponent:New("c2s.Chat.Notice", {
        msg_type = request.msg_type,
        message = request.message
    })
    gateComponent:Call(id, "ChatComponent.Chat", chatMessage)
    gateComponent:BroadCast("ChatComponent.Chat", chatMessage)
    return 0
end

ChatService.Test = function(id, request)
    --coroutine.sleep(0.5)
    Log.Error(id, Json.Encode(request))
    return XCode.Successful, request
end

return ChatService