TxtUtil = {}
require "Util.JsonUtil"
TxtUtil.ReadJsonFile = function(path)
    local file = io.open(path, 'r')
    if file ~= nil then
        local str = file:read("*all")
        return JsonUtil.Decode(str)
    end
end