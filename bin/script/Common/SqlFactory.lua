local SqlFactory = {
    tab = nil,
    result = { }
}

local json = require("util.json")
local table_insert = table.insert
local table_concat = table.concat
local string_format = string.format

function SqlFactory:GetTable(tab)
    self.tab = tab
    self.result = { }
    return self
end

function SqlFactory:Begin()
    table_insert(self.result, string_format("CREATE TABLE IF NOT EXISTS %s(", self.tab))
    return self
end

function SqlFactory:End()
    table_insert(self.result, ")")
    return self
end

function SqlFactory:Insert(document)
    local keys = { }
    local values = { }
    for k, v in pairs(document) do
        table_insert(keys, k)
        if type(v) == "table" then
            local str = json.encode(v)
            table_insert(values, string_format("'%s'", str))
        elseif type(v) == "string" then
            table_insert(values, string_format("'%s'", v))
        elseif type(v) == "boolean" then
            table_insert(values, v and "TRUE" or "FALSE")
        else
            table_insert(values, tostring(v))
        end
    end
    local key1 = table_concat(keys, ',')
    local value1 = table_concat(values, ',')
    table_insert(self.result, string_format("INSERT INTO %s (%s)VALUES(%s)", self.tab, key1, value1))
    return self
end

function SqlFactory:Select(fields)
    if fields == nil then
        table_insert(self.result, string_format("SELECT * FROM %s", self.tab))
        return self
    end
    local field = table_concat(fields, ',')
    table_insert(self.result, string_format("SELECT %s FROM %s", field, self.tab))
    return self
end

function SqlFactory:Filter(filter)
    if type(filter) == "string" then
        table_insert(self.result, string_format(" WHERE %s", filter))
    elseif type(filter) == "table" then
        local filters = { }
        for key, value in pairs(filter) do
            local result = value
            if type(value) == "boolean" then
                result = value and "TRUE" or "FALSE"
            elseif type(value) == "string" then
                result = string_format("'%s'", result)
            elseif type(value) == "table" then
                local str = json.encode(value)
                result = string_format("'%s'", str)
            end
            table_insert(filters, string_format("%s=%s", key, result))
        end
        local where = table_concat(filters, " AND ")
        table_insert(self.result, string_format(" WHERE %s", where))
    end
    return self
end

function SqlFactory:Update(document)
    local documents = { }
    for key, value in pairs(document) do
        local result = value
        if type(value) == "boolean" then
            result = value and "TRUE" or "FALSE"
        elseif type(value) == "string" then
            result = string_format("'%s'", result)
        elseif type(value) == "table" then
            local str = json.encode(value)
            result = string_format("'%s'", str)
        end
        table_insert(documents, string_format("%s=%s", key, result))
    end
    local updater = table_concat(documents, " AND ")
    table_insert(self.result, string_format("UPDATE %s SET %s", self.tab, updater))
    return self
end

function SqlFactory:Delete()
    table_insert(self.result, string_format("DELETE FROM %s", self.tab))
    return self
end

function SqlFactory:Count()
    table_insert(self.result, string_format("SELECT COUNT(*) AS count FROM  %s", self.tab))
    return self
end

function SqlFactory:Limit(limit)
    if limit and limit > 0 then
        table_insert(self.result, string_format(" LIMIT %d", limit))
    end
    return self
end

function SqlFactory:OrderBy(sort)
    if type(sort) == "string" then
        table_insert(self.result, string_format(" ORDER BY %s DESC", sort))
    elseif type(sort) == "table" then
        for field, sorter in pairs(sort) do
            table_insert(self.result, string_format(" ORDER BY %s %s", field, sorter))
            break
        end
    end
    return self
end

function SqlFactory:Next(cc)
    table_insert(self.result, cc or ',')
    return self
end

function SqlFactory:Comment(comment)
    table_insert(self.result, string_format(" COMMENT '%s'", comment))
    return self
end

function SqlFactory:Engine(engine)
    table_insert(self.result, string_format(" ENGINE = %s", engine or "InnoDB"))
    return self
end

function SqlFactory:Character(character)
    table_insert(self.result, string_format(" CHARACTER SET = %s", character or "utf8mb4"))
    return self
end

function SqlFactory:Collate(character)
    table_insert(self.result, string_format(" COLLATE = %s", character or "utf8mb4_unicode_ci"))
    return self
end

function SqlFactory:Unique()
    table_insert(self.result, " UNIQUE")
    return self
end

function SqlFactory:NotNull()
    table_insert(self.result, " NOT NULL")
    return self
end

function SqlFactory:Index(field)
    table_insert(self.result, string_format(" INDEX idx_%s(%s)", field, field))
    return self
end

function SqlFactory:Default(value)
    if type(value) == "string" then
        table_insert(self.result, string_format(" DEFAULT '%s'", value))
    elseif type(value) == "number" then
        table_insert(self.result, string_format(" DEFAULT %d", value))
    elseif type(value) == "boolean" then
        table_insert(self.result, string_format(" DEFAULT %s", value and "TRUE" or "FALSE"))
    end
    return self
end

function SqlFactory:PrimaryKey(keys)
    local val = table.concat(keys, ',')
    table_insert(self.result, string_format("PRIMARY KEY (%s)", val))
    return self
end

function SqlFactory:Int()
    table_insert(self.result, string_format(" INT"))
    return self
end

function SqlFactory:Bool()
    table_insert(self.result, string_format(" BOOL"))
    return self
end

function SqlFactory:Text()
    table_insert(self.result, string_format(" TEXT"))
    return self
end

function SqlFactory:Blob()
    table_insert(self.result, string_format(" INT"))
    return self
end

function SqlFactory:BLOB()
    table_insert(self.result, string_format(" INT"))
    return self
end

function SqlFactory:Json()
    table_insert(self.result, string_format(" JSON"))
    return self
end

function SqlFactory:Float()
    table_insert(self.result, string_format(" FLOAT"))
    return self
end

function SqlFactory:Double()
    table_insert(self.result, string_format(" DOUBLE"))
    return self
end

function SqlFactory:BigInt()
    table_insert(self.result, string_format(" BIGINT"))
    return self
end

function SqlFactory:LongText()
    table_insert(self.result, string_format(" LONGTEXT"))
    return self
end

function SqlFactory:LongBlob()
    table_insert(self.result, string_format(" LongBlob"))
    return self
end

function SqlFactory:AddColumn(field)
    table_insert(self.result, field)
    return self
end

function SqlFactory:VarChar(len)
    table_insert(self.result, string_format(" VARCHAR(%d)", len or 128))
    return self
end

function SqlFactory:Decimal(max, des)
    table_insert(self.result, string_format(" DECIMAL(%d,%d)", max, des))
    return self
end

function SqlFactory:ToString()
    return table_concat(self.result)
end


return SqlFactory


