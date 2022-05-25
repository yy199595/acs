
ChatService = {}

ChatService.Chat = function(id, request, response)
   -- coroutine.sleep(2000)
    local mysqlService = App.GetComponent("MysqlService")

    local request = {}


    request.table = "db_account.tab_user_account"
    request.where_json = Json.Encode({user_id = 1996})
    local code, response = mysqlService:Call("127.0.0.1:7788", "Query", request)

    local json = response.jsonArray[1]
    local userInfo = Json.Decode(json)
    for i, v in pairs(userInfo) do
        Log.Error(i, v)
    end

    local gateComponent = App.GetComponent("GateProxyComponent")
    local chatMessage = {}
    chatMessage.msg_type = request.msg_type
    chatMessage.message = request.message
    gateComponent:BroadCast("ChatComponent.Chat", "c2s.Chat.Notice", Json.Encode(chatMessage))
    return 0
end