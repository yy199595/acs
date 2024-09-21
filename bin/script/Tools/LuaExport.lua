
local LuaExport = { }

function LuaExport.Run(documents)
    local result = "return \n"
    return result .. table.serialize(documents, 1) .. "\n"
end

return LuaExport