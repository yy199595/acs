
Service = {}

function Service.Call(func, id, json)
    local request = Json.Decode(json)
    local state, code, response = pcall(func, id, request)

    if not state then
        Log.Error(code)
        return XCode.CallLuaFunctionFail
    end
    assert(type(code) == 'number')
    assert(type(response) == 'table' or type(response) == 'nil')

    if code ~= XCode.Successful then
        return code
    end
    return code, response ~= nil and Json.Decode(response) or nil
end

function Service.CallAsync(func, id, json)
    Log.Warning(func, id, json)
    local context = function(luaTaskSource)
        local response = {}
        local request = Json.Decode(json)
        local state, error = pcall(func, id, request, response)
        if not state then
            Log.Error(error)
            return luaTaskSource:SetResult(XCode.CallLuaFunctionFail, error)
        end
        assert(type(error) == 'number')
        assert(type(response) == 'table' or type(response) == 'nil')

        if error == XCode.Successful then
            if type(response) == 'table' then
                local str = Json.Encode(response)
                return luaTaskSource:SetResult(error, str)
            end
        end
        return luaTaskSource:SetResult(error, StringUtil.Empty)
    end
    local taskSource = LuaServiceTaskSource.New();
    coroutine.start(context, taskSource)
    return taskSource;
end