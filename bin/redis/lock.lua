local lock = {}
lock.lock = function(tab)
    if redis.call("SETNX", tab.key, 1) == 0 then
        return 0
    end
    return redis.call("SETEX", tab.key, tonumber(tab.time), 1)
end

local tab = {}
for i, v in ipairs(KEYS) do
    tab[v] = ARGV[i]
end

return lock[tab.func](tab)