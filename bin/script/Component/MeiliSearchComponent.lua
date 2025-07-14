
local app = require("App")
local http = require("HttpComponent")
local str_format = string.format
local config = app:GetConfig("ms")

local ADDRESS = config.address
local MeiliSearchComponent = { }

local header = {
    Authorization = str_format("Bearer %s", config.key)
}

---@param tab string
---@return boolean
function MeiliSearchComponent:Create(tab, primaryKey)
    assert(primaryKey)
    local url = str_format("%s/indexes", ADDRESS)
    local response = http:Do("POST", url, header, {
        uid = tab,
        primaryKey = primaryKey
    })
    if response == nil or response.code ~= 200 then
        return nil
    end
    return response.body
end

---@param tab string
---@param setting table
---@return boolean
function MeiliSearchComponent:Setting(tab, setting)
    local url = str_format("%s/indexes/%s/settings", ADDRESS, tab)
    return http:Do("PUT", url, header, {
        searchableAttributes = setting.search, --搜索字段 数组
        filterableAttributes = setting.filter, --过滤字段 数组
        sortableAttributes = setting.sort,    --排序字段 数组
        distinctAttribute = setting.distinct, --去重字段
        rankingRules = setting.ranking --排序规则
    })
end

---@param tab string
---@param document table
---@return table
function MeiliSearchComponent:Set(tab, document)
    local url = str_format("%s/indexes/%s/documents", ADDRESS, tab)
    return http:Do("POST", url, header, document)
end

---@param tab string
---@param query table
---@return table
function MeiliSearchComponent:Search(tab, query)

    local url = str_format("%s/indexes/%s/search", ADDRESS, tab)
    local response = http:Do("POST", url, header, {
        q = query.q,
        filter = query.filter,
        limit = query.limit or 100,
        offset = query.offset or 0,
        sort = query.sort,
        attributesToRetrieve = query.fields
    })
    if response == nil or response.code ~= 200 then
        return nil
    end
    return response.body
end

---@param tab string
---@param uid string | number
---@return table
function MeiliSearchComponent:Find(tab, uid)
    local url = str_format("%s/indexes/%s/documents/%s", ADDRESS, tab, uid)
    return http:Do("POST", url, header, document)
end

---@param taskUid number
function MeiliSearchComponent:Tasks(taskUid)
    local url = str_format("%s/tasks/%s", ADDRESS, taskUid)
    return http:Do("GET", url, header)
end

---@param tab string
---@param filter table
---@return boolean
function MeiliSearchComponent:Delete(tab, filter)
    local url = str_format("%s/indexes/%s/documents", ADDRESS, tab)
    local response = http:Do("DELETE", url, header, filter)
    if response == nil or response.code ~= 200 then
        return false
    end
    return true
end


return MeiliSearchComponent