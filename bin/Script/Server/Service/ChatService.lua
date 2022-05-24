
ChatService = {}

ChatService.Chat = function(id, request, response)
   -- coroutine.sleep(2000)
    local mysqlService = App.GetComponent("GateService")
    Log.Error(mysqlService)
    local code = mysqlService:Call("127.0.0.1:7788", "Ping")
    local gateComponent = App.GetComponent("GateProxyComponent")
    local chatMessage = {}
    chatMessage.msg_type = request.msg_type
    chatMessage.message = request.message
    gateComponent:BroadCast("ChatComponent.Chat", "c2s.Chat.Notice", Json.Encode(chatMessage))
    return 0
end