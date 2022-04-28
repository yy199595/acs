local lock = {}
lock.lock = function(tab)
    if redis.call("SETNX", tab.key, 1) == 0 then
        print("redis lock " .. tab.key .. "set faulure")
        return 0
    end
    return redis.call("SETEX", tab.key, tonumber(tab.time), 1)
end

lock.unlock = function(tab)
    tab.time = tonumber(tab.time)
    local tab1 = {}
    for i, v in pairs(tab) do
        table.insert(tab1, 1)
        table.insert(tab1, 2)
    end
    return tab1
end

local tab = {}
for i, v in ipairs(KEYS) do
    tab[v] = ARGV[i]
end

return lock[tab.func](tab)