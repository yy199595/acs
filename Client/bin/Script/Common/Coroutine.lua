function coroutine.wakeup(cor, ...)
    local res, ret = coroutine.resume(cor, ...)
    assert(res, ret)
    return ret
end

function coroutine.start(func, ...)
    local cor = coroutine.create(func)
    return coroutine.wakeup(cor, ...)
end
