
ChatService = {}

ChatService.Chat = function(id, request, response)
    coroutine.sleep(2000)
    local gateComponent = App.GetComponent("GateProxyComponent")

    local chatMessage = {}
    chatMessage.msg_type = request.msg_type
    chatMessage.message = request.message
    gateComponent:BroadCast("ChatComponent.Chat", "c2s.Chat.Notice", Json.ToString(chatMessage))
    return 0
end