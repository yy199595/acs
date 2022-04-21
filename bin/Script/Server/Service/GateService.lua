
GateService = {}

GateService.Ping = function(id, request, response)
    Log.Error(id, " ping")
    return XCode.Successful
end