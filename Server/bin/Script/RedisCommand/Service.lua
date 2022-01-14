
local Service = {}

Service.Add = function(keys)
    local id = keys[1]
    local services = {}
    local address = keys[2]
    for index = 3, #keys do
        table.insert(services, keys[index])
    end

    local count = 0
    for index = 3, #keys do
        local service = keys[index]
        redis.call('SADD', address, service)
        local key = tostring(id) .. ':' .. service
        count = count + redis.call('SADD', key , address)
    end

    local str = table.concat(services, ",");
    print(string.format("[%s] add %d service : (%s)", address, count, str))
    return count
end


Service.Get = function(array)
    local id = array[1]
    local service = array[2]
    local key = tostring(id) .. ':' .. service
    local services = redis.call('SINTER', key)
    if services == nil then
        local key1 = "0:"..service
        return redis.call('SINTER', key1)
    end
    return services
end

Service.Remove = function(keys)
    local count = 0
    local id = keys[1]
    local address = keys[2]
    local services = redis.call('SMEMBERS', address)
    for _, name in ipairs(services) do
        local key = tostring(id) .. ':' .. name
        count = count + redis.call('SREM', key, address)
    end
    print("remove ", address, " count = ", count)
    redis.call('SREM', address, table.concat(services, ' '))
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