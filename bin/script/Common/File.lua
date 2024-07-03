local file = require("app.file")

local File = { }

---@param filePath string
---@return string
function File:GetFileName(filePath)
    return filePath:match("[^/]+$")
end

---@param dir string
---@param fmt string
---@return table
function File:GetAllFile(dir, fmt)
    return file.Find(dir, fmt)
end

---@param filePath string
---@return string
function File:ReadAll(filePath)
    local fs = io.open(filePath, "r")
    if fs == nil then
        return
    end
    local content = fs:read("*all")
    fs:close()
    return content
end

---@param filePath string
---@return string
function File:GetMd5(filePath)
    return file.GetMd5(filePath)
end

return File