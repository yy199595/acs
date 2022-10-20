
MessageComponent = {}
local self = App.GetComponent("ProtoComponent")
function MessageComponent.Awake()
    print(self.Import)
    self:Import("message/s2s.proto")
end

function MessageComponent.Hotfix()
    self:Import("s2s.proto")
end

function MessageComponent.New(name, data)
    assert(type(name) == "string")
    if type(data) == "table" then
        return self:NewByTable(data)
    end
    if type(data) == "string" then
        return self:NewJson(data)
    end
    return nil
end
return MessageComponent