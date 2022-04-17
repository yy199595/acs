
local Account = {}
Account.AddNewUser = function(keys)
    local account = keys[1]
    local ret = redis.call("SADD", "user_account", account)
    if ret == 0 then
        return 0
    end
    return redis.call("INCR", "user_id") + 1995;
end
local args = {}
for index = 2, #KEYS do
    table.insert(args, KEYS[index])
end

return Account[KEYS[1]](args)