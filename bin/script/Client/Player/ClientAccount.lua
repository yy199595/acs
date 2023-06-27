
local accounts = { }
local ACCOUNT_COUNT = 1
for i = 10000, 10000 + ACCOUNT_COUNT do
    local tab = {
        account = string.format("%s@qq.com", i),
        passwd = tostring(i + os.time()),
        phone_num = 13716061990 + i
    }
    table.insert(accounts, tab)
end

return accounts