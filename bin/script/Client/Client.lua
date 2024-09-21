local log = require("Log")
local json = require("util.json")
local str_format = string.format
local timer = require("core.timer")


local Main = { }

function Main:OnAwake()
    timer.AddUpdate(self, "OnUpdate")
end

function Main:OnUpdate()
    print("=============")
end

function Main:OnComplete()

end

return Main