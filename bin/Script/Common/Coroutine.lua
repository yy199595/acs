function coroutine.wakeup(cor, ...)
    local res, ret = coroutine.resume(cor, ...)
    assert(res, ret)
    return ret
end

function coroutine.sleep(ms)
    local co = coroutine.running()
    Timer.AddTimer(ms, co)
    coroutine.yield()
end

function coroutine.call(func)
    local luaTaskSource = LuaTaskSource.New()
    coroutine.start(function(taskSource)
        local state, ret = pcall(func)
        if not state then
            Log.Error(ret)
        end
        taskSource:SetResult(ret)
    end, luaTaskSource)
    return luaTaskSource
end

-- function coroutine.sleep(ms)
--     if coroutine.running() == nil then
--         assert(false, "sleep not in coroutine")
--     end
--     return SoEasy.Sleep(ms)
-- end
