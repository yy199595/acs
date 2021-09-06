
Service = {}
function Service.Invoke(method, response, message, id, json)
    print("service :", response, message, method, id, json)
    local request = nil
    if type(json) == 'string' then
        request = JsonUtil.ToObject(json)
    end
    local code, res = method(id, request)
    if code == XCode.Successful then
        res = JsonUtil.ToString(res)
    end
    return response(message, code, res)
end