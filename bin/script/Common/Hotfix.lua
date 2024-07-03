
local Hotfix = { }

function Hotfix:__Hotfix()

    assert(self.__source, self.__name)
    local new_module = dofile(self.__source)
    for k, v in pairs(new_module) do
        if type(v) == "function" then
            self[k] = v
        end
    end
    for k, v in pairs(self) do
        if type(v) == "function" and new_module[k] == nil then
            self[k] = nil
        end
    end
end

return Hotfix