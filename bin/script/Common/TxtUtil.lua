local TextReader = { }

function TextReader:LoadCsv(path)
    local file = io.open(path)
    local content = file:lines()
    for line in file:lines() do

    end
end

function TextReader:ReadLines(path)
    local lines = { }
    local fs = io.open(path)
    for line in fs:lines() do
        table.insert(lines, line)
    end
    return lines
end

return TextReader