
ProtoComponent = {}
local self = App.GetComponent("ProtocolComponent")
function ProtoComponent.Awake()
    assert(self:Import("message/s2s.proto"))
end

function ProtoComponent.Hotfix()
    self:Import("s2s.proto")
end

function ProtoComponent.NewByJson(name, json)
    assert(type(name) == "string")
    assert(type(json) == "string")
    return self:NewJson(name, json)
end

function ProtoComponent.NewByTable(name, tab)
    assert(type(tab) == "table")
    assert(type(name) == "string")
    return self:New(name, tab)
end

return ProtoComponent