
StringUtil = {}

StringUtil.Empty = ""
function StringUtil.SplitString(str, reps)
    local resultStrList = {}
    string.gsub(str,'[^'..reps..']+',function ( w )
        table.insert(resultStrList,w)
    end)
    return resultStrList
end

function StringUtil.ParseAddress(addrss)
    local res = StringUtil.SplitString(addrss, ":")
    if #res == 2 then
        return res[1], tonumber(res[2])
    end
end