
local Gate = Class("RpcService")

function Gate:Awake()
    self.app:AddWatch("Chat")
end

return Gate