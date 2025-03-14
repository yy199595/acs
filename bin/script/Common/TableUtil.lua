
local str_format = string.format
local table_insert = table.insert

function table.print(t)
    print(table.tostring(t))
end

function table.append(source, target)
    for _, v in ipairs(target) do
        table.insert(source, v)
    end
end

function table.find(tab, value)
    for i, v in pairs(tab) do
        if v == value then
            return i, v
        end
    end
    return nil
end

function table.is_array(t)

    local count = 0
    for _, v in pairs(t) do
        count = count + 1
    end
    return count == #t;
end

function table.tostring(t)
    local buffer = { }
    local print_r_cache = {}
    local function sub_print_r(t, indent)
        if (print_r_cache[tostring(t)]) then
            table_insert(buffer, indent .. "*" .. tostring(t))
        else
            print_r_cache[tostring(t)] = true
            if (type(t) == "table") then
                for pos, val in pairs(t) do
                    if (type(val) == "table") then
                        table_insert(buffer, indent .. "[" .. pos .. "] = " .. tostring(t) .. " {")
                        sub_print_r(val, indent .. string.rep(" ", string.len(pos) + 8))
                        table_insert(buffer, indent .. string.rep(" ", string.len(pos) + 6) .. "}")
                    elseif (type(val) == "string") then
                        table_insert(buffer,indent .. "[" .. pos .. '] = "' .. val .. '"')
                    else
                        table_insert(buffer,indent .. "[" .. pos .. "] = " .. tostring(val))
                    end
                end
            else
                table_insert(buffer,indent .. tostring(t))
            end
        end
    end
    if (type(t) == "table") then
        table_insert(buffer,tostring(t) .. "\n{")
        sub_print_r(t, "  ")
        table_insert(buffer,"}")
    else
        sub_print_r(t, "  ")
    end
    return table.concat(buffer, "\n")
end

--function table.serialize(t)
--    local result = "{"
--    local is_array = table.is_array(t)
--
--    for k, v in pairs(t) do
--        local key, value
--        if is_array then
--            -- 如果是数组，只输出值
--            value = (type(v) == "table") and table.serialize(v) or (type(v) == "string" and str_format("%q", v) or tostring(v))
--            result = result .. value .. ","
--        else
--            -- 如果是键值对表，输出键和值
--            key = (type(k) == "string") and str_format("[%q]", k) or str_format("[%s]", k)
--            value = (type(v) == "table") and table.serialize(v) or (type(v) == "string" and str_format("%q", v) or tostring(v))
--            result = result .. "  " .. key .. "=" .. value .. ","
--        end
--    end
--
--    result = result .. "}"
--    return result
--end
--
--function table.deserialize(str)
--    local code = "return " .. str
--    local ok, result = pcall(load, code)
--    if not ok then
--        return nil
--    end
--    return result()
--end
