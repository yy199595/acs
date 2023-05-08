
Class = {}

function Class.Clone(meta)
    local tab = {}
    setmetatable(tab, { __index = meta })
    return tab
end