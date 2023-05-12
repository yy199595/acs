local proto = Proto

local ProtoHelper = { }

function ProtoHelper.New(name, tab)
    return proto.New(name, tab)
end

function ProtoHelper.Import(path)
    return proto.Import(path)
end

function ProtoHelper.Encode(name, data)
    return proto.Encode(name, data)
end

function ProtoHelper.Decode(name, message)
    return proto.Decode(name, message)
end
return ProtoHelper