TxtUtil = {}
TxtUtil.ReadJsonFile = function(path)
    local file = io.open(path, 'r')
    if file ~= nil then
        local str = file:read("*all")
        return JsonUtil.ToObject(str)
    end
end

return TxtUtil