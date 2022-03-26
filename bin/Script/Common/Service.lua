
Service = {}

function Service.Call(func, id, json)

    local tab = #json > 0 and Json.ToObject(json) or nil
    local state, error, response = pcall(func, id, tab)

    if not state then
        Log.Error(error)
        return XCode.CallLuaFunctionFail
    end
    assert(type(error) == 'number')
    assert(type(response) == 'table' or type(response) == 'nil')

    if error ~= XCode.Successful then
        return code
    end
    return code, Json.ToObject(response)
end

function Service.CallAsync(func, id, json)

    print(coroutine.running())
    local context = function(luaTaskSource)
        local tab = #json > 0 and Json.ToObject(json) or nil
        local state, error, response = pcall(func, id, tab)
        if not state then
            Log.Error(error)
            return luaTaskSource:SetResult(XCode.CallLuaFunctionFail, error)
        end
        assert(type(error) == 'number')
        assert(type(response) == 'table' or type(response) == 'nil')

        if error == XCode.Successful then
            if type(response) == 'table' then
                local str = Json.ToString(response)
                print("response = ", str)
                return luaTaskSource:SetResult(error, str)
            end
        end
        return luaTaskSource:SetResult(error, StringUtil.Empty)
    end
    local taskSource = LuaServiceTaskSource.New();
    coroutine.start(context, taskSource)
    return taskSource;
end