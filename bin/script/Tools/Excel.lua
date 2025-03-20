
os.execute("chcp 65001")
local lfmt = require("util.fmt")
local log = require("Console")
local app = require("core.app")
local fs = require("util.fs")
local lxlsx = require("util.lxlsx")
local serverConfig = app.GetConfig()
local config = serverConfig["excel"]
local Module = require("Module")
local Excel = Module()

Excel.Handler = { }
Excel.Line = {
    CommentLine = 3, --注释行
    ValueTypeLine = 2, --字段类型行
    FieldNameLine = 1, --字段名字行
    ValueStartLine = 4, --数据起始行
    ValueStartCol = 1 --数据起始列
}

function Excel:IsIgnore(str)
    for _, tag in ipairs(config.ignore) do
        if string.find(str, tag) then
            return true
        end
    end
    return false
end

function Excel:IsValid(str)
    return str:match("^%w+$") ~= nil
end
-- 40%
function GetProcess(index, sum)
    local result = { }
    local process = (index / sum) * 100
    table.insert(result, lfmt.format(" {:.2f}% ", process))
    for i = 1, math.floor(process) do
        table.insert(result, "=")
    end
    return table.concat(result, "")
end

function Excel:OnAwake()
    self.output = config.output
    self.Handler["cs"] = require("CsExport")
    self.Handler["cpp"] = require("CppExport")
    self.Handler["lua"] = require("LuaExport")
    self.Handler["json"] = require("JsonExport")


    local files = fs.GetFiles(config.dir, "xlsx")
    if files == nil  then
        log.Error("[{}]目录不存在", config.dir)
        return
    end
    if #files == 0 then
        log.Error("该目录不存在xlsx文件")
        return
    end
    for _, output in ipairs(self.output) do
        if output.path == nil then
            log.Error("没有设置文件输出路径")
            return
        end
        fs.MakeDir(output.path)
        if self.Handler[output.type] == nil then
            log.Error("找不到导出处理函数: {}", output.type)
            return
        end
        self.Handler[output.type].config = output
    end
    local sum = #files
    local t1 = time.ms()
    for idx, path in ipairs(files) do
        if self:IsIgnore(path) then
            goto continue
        end
        local ok, excelFile = pcall(lxlsx.new, path)
        if not ok then
            log.Error(excelFile)
        else
            local process = idx / sum
            local name = fs.GetFileName(path)
            self:OnReadFile(excelFile, name, process)
        end
        :: continue ::
    end
    log.Debug("导出总耗时 => {:.2f}s", (time.ms() - t1) / 1000.0)
    os.exit()
end

function Excel:DecodeByType(type, value)
    local decoder = require("TypeDecode")
    local handler = decoder[type]
    if handler == nil then
        return value
    end
    return handler(value)
end

function Excel:OnReadSheet(sheet, fileName)

    local types = {}
    local fields = {}
    local descs = { }
    local name = sheet:get_name()
    local last_row = sheet:get_last_row()
    local last_col = sheet:get_last_col()

    for y = self.Line.ValueStartCol, last_col do
        local cell = sheet:get_cell(self.Line.FieldNameLine, y)

        if not cell then
            goto continue
        end
        local value = cell:get_value()
        if not value and #value <= 0 then
            error(string.format("文件:%s 分页:%s %s行数据不合法", fileName, name, y))
        end
        if self:IsIgnore(value) then
            goto continue
        end
        local cell1 = sheet:get_cell(self.Line.CommentLine, y)
        if cell1 then
            descs[value] = cell1:get_value()
        end
        if value ~= "" then
            fields[y] = value
        end
        ::continue::
    end

    for y = self.Line.ValueStartCol, last_col do
        local cell = sheet:get_cell(self.Line.ValueTypeLine, y)
        if not cell then
            goto continue
        end
        local value = cell:get_value()
        if not value and #value <= 0 then
            log.Error("文件:{} 分页:{} {}行数据不合法", fileName, name, y)
        end
        types[y] = value
        ::continue::
    end
    local t1 = time.ms()
    local documents = {}
    for x = self.Line.ValueStartLine, last_row do
        local item = {}
        for y = self.Line.ValueStartCol, last_col do
            local cell = sheet:get_cell(x, y)
            if cell then
                local value = cell:get_value()
                if value and #value > 0 then
                    local type = types[y]
                    local filed = fields[y]
                    if filed == nil or filed == "" then
                        goto continue
                    end
                    local target = self:DecodeByType(type, value)
                    if target == nil then
                        log.Error("file:{} x:{} y:{}", fileName, x, y)
                        return false
                    end
                    rawset(item, filed, target)

                end
            end
            ::continue::
        end
        if os.SetConsoleTitle then
            os.SetConsoleTitle(lfmt.format("[{}]  {:.2f}%", fileName, (x / last_row) * 100))
        end

        if next(item) then
            table.insert(documents, item)
        end
    end
    log.Debug("[{}ms] 读取{} 成功共({})行", time.ms() - t1, fileName, #documents)
    local result = { }
    for _, output in ipairs(self.output) do
        local handler = self.Handler[output.type]
        if handler == nil then
            log.Error("找不到导出处理函数:%s", output.type)
            return
        end
        local t1 = time.ms()
        if result[output] == nil then
            result[output] = handler.Run(documents, types, fields, name, descs)
        end
        local content = result[output]
        if content == nil or content == "" then
            return false
        end
        local dir = output.path
        local path = string.format("%s/%s%s", dir, name, output.ext)
        if not fs.Write(path, content) then
            log.Error("打开文件[{}]失败", path)
            return false
        end
        log.Info("[{}ms] 导出文件{}到{}成功", (time.ms() - t1), name, path)
    end

    return true
end

function Excel:OnReadFile(excelFile, fileName, process)
    local sheets = excelFile:GetSheets()
    for _, sheet in ipairs(sheets) do
        local name = sheet:get_name()
        if self:IsIgnore(name) then
            goto continue
        end
        if not self:IsValid(name) then
            goto continue
        end
        local t1 = time.ms()
        if self:OnReadSheet(sheet, fileName) then
            local t2 = time.ms() - t1
            log.Debug("[{}ms] ({:.2f}%) 导出文件分页[{}:{}]成功", t2, process * 100, fileName, name)
        else
            log.Error("导出文件分页[{}:{}]失败", fileName, name)
        end
        :: continue ::
    end
end

return Excel