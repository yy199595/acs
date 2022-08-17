
ProtoComponent = {}
local messageComponent = App.GetComponent("MessageComponent")
function ProtoComponent.Awake()
    assert(messageComponent:Import("message/s2s.proto"))
end

function ProtoComponent.Hotfix()
    messageComponent:Import("s2s.proto")
end

function ProtoComponent.NewByJson(name, json)
    assert(type(name) == "string")
    assert(type(json) == "string")
    return messageComponent:NewJson(name, json)
end

function ProtoComponent.NewByTable(name, tab)
    assert(type(tab) == "table")
    assert(type(name) == "string")
    return messageComponent:New(name, tab)
end

return ProtoComponent