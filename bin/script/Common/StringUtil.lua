
local math_floor = math.floor
function string.split(str, reps)
    local resultStrList = {}
    string.gsub(str,'[^'..reps..']+',function ( w )
        table.insert(resultStrList,w)
    end)
    return resultStrList
end

function string.addr(address)
    local res = string.split(address, ":")
    if #res == 2 then
        return res[1], tonumber(res[2])
    end
end

---@param str string
---@return number
function string.hash(str)
    local hash = 0
    local prime = 1099511628211 -- 一个大素数，适用于 64 位哈希计算

    for i = 1, #str do
        local char_code = string.byte(str, i)
        hash = (hash * prime + char_code) % (2^64) -- 限制在 64 位范围
    end

    return math_floor(hash)
end