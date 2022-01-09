
CenterHostService = {}

function CenterHostService.Add(id, request)
    local json = Json.ToString(request)
    print("request json = " , type(request), coroutine.running())
    coroutine.sleep(1000)
    return XCode.Successful,
    {
        globalId = 10
    }
end