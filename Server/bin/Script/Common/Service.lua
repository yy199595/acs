
Service = {}
function Service.Invoke(method, response, message, id, json)
    local request = nil
    if type(json) == 'string' then
        request = Json.ToObject(json)
    end
    local code, res = method(id, request)
    if code == XCode.Successful and res then
        res = Json.ToString(res)
    end
    return response(message, code, res)
end

return Service