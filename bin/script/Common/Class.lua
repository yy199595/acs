
Class = {}

function Class.Clone(meta)
    local tab = {}
    setmetatable(tab, meta)
    return tab
end