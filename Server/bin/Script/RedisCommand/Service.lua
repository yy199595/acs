
local Service = {}

local Json = {}
function Json.ToString(t)
    local function serialize(tbl)
        local tmp = {}
        for k, v in pairs(tbl) do
            local k_type = type(k)
            local v_type = type(v)
            local key = (k_type == "string" and "\"" .. k .. "\":")
                    or (k_type == "number" and "")
            local value = (v_type == "table" and serialize(v))
                    or (v_type == "boolean" and tostring(v))
                    or (v_type == "string" and "\"" .. v .. "\"")
                    or (v_type == "number" and v)
            tmp[#tmp + 1] = key and value and tostring(key) .. tostring(value) or nil
        end
        if table.maxn(tbl) == 0 then
            return "{" .. table.concat(tmp, ",") .. "}"
        else
            return "[" .. table.concat(tmp, ",") .. "]"
        end
    end
    assert(type(t) == "table")
    return serialize(t)
end

local AddFunction = "NodeAddressService.Add"

Service.Add = function(keys)
    local id = keys[1]
    local services = {}
    local address = keys[2]
    for index = 3, #keys do
        table.insert(services, keys[index])
    end

    local ret = {}
    ret.services = {}
    ret["address"] = address
    ret["area_id"] = tonumber(id)

    local count = 0
    for index = 3, #keys do
        local service = keys[index]
        table.insert(ret.services, service)
        redis.call('SADD', address, service)
    end

    redis.call("SADD", "area_" .. id, address)

    local json = Json.ToString(ret)
    redis.call("PUBLISH", AddFunction, json)
    print("redis publish [" .. AddFunction .. "] json = ", json)
    return count
end


Service.Get = function(array) -- 区服拿到地址  地址拿到服务
    local id = array[1]

    local ret = {}
    local key = "area_" .. id
    local addressArray = redis.call("SMEMBERS", key)
    for _, address in ipairs(addressArray) do
        local tab = {}
        tab.address = address
        tab.area_id = tonumber(id)
        tab.services = redis.call("SMEMBERS", address)
        table.insert(ret, Json.ToString(tab))
    end
    return ret
end

Service.Remove = function(keys)
    local count = 0
    local id = keys[1]
    redis.call("SREM", "area_".. id, address)
    local items = redis.call("SMEMBERS", address)
    for _, item in ipairs(items) do
        count = count + redis.call("SREM", address, item)
    end
    return count
end

if #KEYS < 1 then
    return "args number min = 2"
end

print("call function ", KEYS[1])

local args = {}
for index = 2, #KEYS do
    table.insert(args, KEYS[index])
end

return Service[KEYS[1]](args)