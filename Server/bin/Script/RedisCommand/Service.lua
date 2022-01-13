
local Service = {}

Service.Push = function(keys)
    local key = "rpc.services"
    assert(type(keys) == 'table')
    print(table.concat(keys, ","))
    return redis.call('SADD', key, table.concat(keys, ' '))
end

Service.Add = function(keys)
    local id = keys[1]
    local service = keys[2]
    local address = keys[3]
    local key = string.format("%d:rpc.%s",id, service)
    print(string.format("add new service [%s] %s %s",key, service, address))
    return redis.call('SADD', key , address)
end

Service.Get = function(array)
    local id = array[1]
    local service = array[2]
    local key = string.format("%d:rpc.%s", id, service)
    local services = redis.call('SINTER', key)
    if services == nil then
        key = string.format("0:rpc.%s", id, service)
        return redis.call('SINTER', key)
    end
    return services
end

Service.Remove = function(keys)
    local count = 0
    local id = keys[1]
    local address = keys[2]
    local services = redis.call('SMEMBERS','rpc.services')
    for _, name in ipairs(services) do
        local key = string.format("%d:rpc.%s", id, name)
        count = count + redis.call('SREM', key)
    end
    print("remove ", address, " count = ", count)
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