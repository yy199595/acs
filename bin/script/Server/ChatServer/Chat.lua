local Chat = { }
require("XCode")
require("coroutine")
local proto = require("Proto")
function Chat.OnLogin(userId)
    coroutine.sleep(1000)
    print(string.format("玩家%d登录聊天服务", userId))
end

function Chat.Chat(request)
    coroutine.sleep(1000)
    local message = request.message
    local chatMessage = proto.New("c2s.chat.notice", {
        msg_type = message.msg_type,
        message = message.message
    })
    --Gate.Send(id, "ChatComponent.Private", chatMessage)
    Gate.BroadCast("ChatComponent.Chat", chatMessage)
    return XCode.Successful
end
local count = 0

function Chat.Ping(request)
    count = count + 1
    print(os.time(), "count = " .. count)
    return XCode.Successful
end

return Chat