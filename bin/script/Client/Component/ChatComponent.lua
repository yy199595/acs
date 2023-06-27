require("RpcService")

local ChatComponent = Class("RpcService")

function ChatComponent:Chat(request)

end

function ChatComponent:OnLoginSuccess()

end

function ChatComponent:OnUpdate()
    print("===========")
    coroutine.start(self.CallChat, self)
end

function ChatComponent:CallChat()
    self.player:Call("Chat.Chat", {
        msg_type = 1,
        message = "hello"
    })
end

return ChatComponent