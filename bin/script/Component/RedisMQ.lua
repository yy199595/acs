
local RedisMQ = { }
local redis = require("RedisComponent")

---@param mq string
---@param field string
---@param value string
function RedisMQ:Add(mq, field, value)
    return redis:Run("XADD", mq, "*", field, value)
end

---@param mq string
---@param count number
---@param timeout number
function RedisMQ:Read(mq, count, timeout)
    local resp = redis:Run("XREAD", "BLOCK", timeout, "COUNT", count, "STREAMS", mq, 0)
    if type(resp) ~= "table" or #resp == 0 then
        return nil
    end
    local results = { }
    local response = resp[1]
    local key, value = table.unpack(response)
    for _, val in ipairs(value) do
        local item = {
            mq = key,
            id = val[1],
            field = val[2][1],
            value = val[2][2]
        }
        table.insert(results, item)
    end
    return results
end

---@param mq string
---@param id string
function RedisMQ:Del(mq, ...)
    return redis:Run("XDEL", mq, ...)
end

return RedisMQ