
function Class(...)
    local class = { }
    local metas = table.pack(...)
    for _, meta in ipairs(metas) do
        if type(meta) == 'string' then
            meta = require(meta)
        end
        setmetatable(class, meta)
    end
    return class
end
