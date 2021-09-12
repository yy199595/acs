
Service = {}
function Service.Invoke(method, response, message, id, json)
    print("service :", response, message, method, id, json)
    local request = nil
    if type(json) == 'string' then
        request = Json.ToObject(json)
    end
    local code, res = method(id, request)
    print("response = ",code, res)
    if code == XCode.Successful then
        res = Json.ToString(res)
    end
    return response(message, code, res)
end