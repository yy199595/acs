
require("TableUtil")
local log = require("Log")
local this = require("net.http")
local tab_concat = table.concat
local str_format = string.format
local HttpComponent =  { }

function HttpComponent:FormatUrl(url, query)
    if not query then
        return url
    end
    local t = type(query)
    if t == "string" and #query > 0 then
        return str_format("%s?%s", url, query)
    end
    if t == "table" and next(query) then
        local param = { }
        for k, v in pairs(query) do
            table.insert(param, str_format("%s=%s", k, v))
        end
        return str_format("%s?%s", url, tab_concat(param, "&"))
    end
end

---@param url string
---@param sync boolean
---@return table
function HttpComponent:Get(url, query, sync)
    local full_url = self:FormatUrl(url, query)
    local response = this.Get(full_url, sync)
    if response.code ~= 200 then
        log.Error("[GET] %s error : %s", full_url, response.status)
        return response
    end
    return response.body
end

---@param url string
---@param data table
---@param sync boolean
---@return table
function HttpComponent:Post(url, data, sync)
    local response = this.Post(url, data, sync)
    if response.code ~= 200 then
        log.Error("[POST] %s error : %s", url, response.status)
        return
    end
    return response.body
end

---@param method string
---@param url string
---@param head table
---@param body any
---@return table
function HttpComponent:Do(method, url, head, body)
    return this.Do(method, url, head, body)
end

---@param url string
---@param path string
function HttpComponent:Download(url, path)
    return this.Download(url, path)
end

---@param url string
---@param path string
---@param content string
---@return table
function HttpComponent:Upload(url, path, content)
    return this.Upload(url, path, content)
end

return HttpComponent