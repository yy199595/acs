---
--- Generated by EmmyLua(https://github.com/EmmyLua)
--- Created by leyi.
--- DateTime: 2023/5/8 20:34
---

local proto = Proto

local ProtoHelper = { }

function ProtoHelper.New(name, tab)
    return proto.New(name, tab)
end

function ProtoHelper.Import(path)
    return proto.Import(path)
end
return ProtoHelper