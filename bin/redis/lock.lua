local lock = {}
lock.lock = function(tab)
    if redis.call("SETNX", tab.key, 1) == 0 then
        return 0
    end
    return redis.call("EXPIRE", tab.key, tonumber(tab.time))
end

for i, v in ipairs(KEYS) do
    tab[v] = ARGV[i]
end

return lock[tab.func](tab)