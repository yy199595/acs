local token = { }
local json_encode = cjson.encode
local json_decode = cjson.decode

token.add = function(request)
    local ip = request.ip
    local t = request.exp_time
    local user_token = request.token

end

token.auth = function()

end

local result = { }
local func = KEYS[1]
local method = user[func]
if type(method) ~= "function" then
    result.error = func .. " not lua function"
else
    local req = ARGV[1]
    local request = json_decode(req)
    local state, err, response = pcall(method, request)
    if not state then
        result.error = err
    else
        result.data = response
    end
end
return json_encode(result)