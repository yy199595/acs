
local coroutine_create = coroutine.create
local coroutine_resume = coroutine.resume

function coroutine.start(fun, ...)
    local co = coroutine_create(fun)
    return coroutine_resume(co, ...)
end
