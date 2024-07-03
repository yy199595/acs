local proto = require("util.proto")

local ProtoHelper = { }

---@param name string
---@param tab table
---@return userdata
function ProtoHelper.New(name, tab)
    return proto.New(name, tab)
end

---@param path string
---@return boolean
function ProtoHelper.Import(path)
    return proto.Import(path)
end

---@param name string
---@param data userdata
---@return string
function ProtoHelper.Encode(name, data)
    return proto.Encode(name, data)
end

---@param name string
---@param message string
---@return userdata
function ProtoHelper.Decode(name, message)
    return proto.Decode(name, message)
end
return ProtoHelper