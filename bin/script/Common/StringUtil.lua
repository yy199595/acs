
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