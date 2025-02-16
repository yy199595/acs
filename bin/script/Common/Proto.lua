local proto = require("util.pb")

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
    return proto.import(path)
end

---@param name string
---@param data userdata
---@return string
function ProtoHelper.Encode(name, data)
    return proto.encode(name, data)
end

---@param data userdata
---@return string
function ProtoHelper.ToJson(data)
    return proto.to_json(data)
end

---@param name string
---@param message string
---@return userdata
function ProtoHelper.Decode(name, message)
    return proto.decode(name, message)
end
return ProtoHelper