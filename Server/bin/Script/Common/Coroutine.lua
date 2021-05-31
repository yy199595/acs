
function coroutine.wakeup(cor, ...)
   local res, ret = coroutine.resume(cor, ...)
   assert(res, ret)
   return ret 
end

function coroutine.start(func, ...)
    local cor = coroutine.create(func)
    return coroutine.wakeup(cor, ...)
end

function coroutine.sleep(ms)
    if coroutine.running() == nil then
        assert(false, "sleep not in coroutine")
    end
    return SoEasy.sleep(ms)
end
