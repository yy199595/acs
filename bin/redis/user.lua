
local user = {}
user.add = function(tab)
    if redis.call("SADD", "user_account", tab.account) == 0 then
        return 0
    end
    return redis.call("INCR", "user_id") + 1995;
end

user.set = function(tab)
    redis.call("SET", tab.token, tonumber(tab.user_id))
    return redis.call("EXPIRE", tab.key, tonumber(tab.time))
end

local tab = {}
for i, _ in ipairs(KEYS) do
    tab[i] = ARGV[i]
end

return user[tab.func](tab)