
ChatService = {}

ChatService.Chat = function(id, request, response)
    --coroutine.sleep(2.5)
    local gateComponent = App.GetComponent("GateProxyComponent")
    local chatMessage = Message.New("c2s.Chat.Notice", {
        msg_type = request.msg_type,
        message = request.message
    })
    gateComponent:Call(id, "ChatComponent.Chat", chatMessage)
    gateComponent:BroadCast("ChatComponent.Chat", chatMessage)
    return 0
end