

local node = assert(require("core.node"))
local actor = assert(require("core.actor"))

local COMPONENT = "NodeComponent"

local Node = { }
local router_send = assert(actor.Send)
local router_call = assert(actor.Call)
local log_err = assert(require("Log").Error)

local node_next = assert(node.next)
local node_rand = assert(node.rand)
local node_hash = assert(node.hash)

local node_create = assert(node.create)
local node_remove = assert(node.remove)
local node_get_info = assert(node.get_info)
local node_get_listen = assert(node.get_listen)
local node_add_listen = assert(node.add_listen)


---@param actorId number
---@param name string
---@param request table
---@return number
function Node:Send(actorId, name, request)
    local status, code = xpcall(router_send, log_err, COMPONENT, actorId, name, request)
    if not status then
        return XCode.CallLuaFunctionFail
    end
    return code
end

---@param actorId number
---@param name string
---@param request table
---@return number table
function Node:Call(actorId, name, request)
    local status, code, response = xpcall(router_call, log_err, COMPONENT, actorId, name, request)
    if not status then
        return XCode.CallLuaFunctionFail
    end
    return code, response
end

---@param name string
---@param request table|string|number
function Node:Broadcast(name, request)

end


---@param id number
---@param name string
function Node:Create(id, name)
    return node_create(id, name)
end

---@param id number
function Node:Remove(id)
    return node_remove(id)
end

---@param id number
function Node:GetInfo(id)
    return node_get_info(id)
end

---@param id number
---@param name string
function Node:GetListen(id, name)
    return node_get_listen(id, name)
end

---@param id number
---@param name string
---@param address string
function Node:AddListen(id, name, address)
    return node_add_listen(id, name, address)
end

---@param service string
---@param type string
---@param hash number
---@return number
function Node:Allot(service, type, hash)
    if type == "rand" then
        return node_rand(service)
    elseif type == "hash" then
        return node_hash(service, hash)
    end
    return node_next(service)
end


return Node