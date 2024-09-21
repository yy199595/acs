
local JsonExport = { }

function JsonExport.Run(documents)
    local json = require("util.json")
    return json.pretty(documents)
end

return JsonExport