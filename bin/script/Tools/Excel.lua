require("TableUtil")
local app = require("App")
local fs = require("util.fs")
local log = require("Console")
local director = require("util.dir")
local config = app:GetConfig("excel")

local Excel = { }
Excel.Handler = { }
Excel.Line = {
    CommentLine = 1, --注释行
    ValueTypeLine = 3, --字段类型行
    FieldNameLine = 2, --字段名字行
    ValueStartLine = 4, --数据起始行
    ValueStartCol = 3 --数据起始列
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

function Excel:Awake()
    self.output = config.output
    self.Handler["cpp"] = require("CppExport")
    self.Handler["lua"] = require("LuaExport")
    self.Handler["json"] = require("JsonExport")

    local files = fs.GetFiles(config.dir, "xlsx")
    if files == nil then
        log.Error("目录不存在或者目录有中文路径")
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
        director.Make(output.path)
        if self.Handler[output.type] == nil then
            log.Error("找不到导出处理函数:%s", output.type)
            return
        end
    end

    for _, path in ipairs(files) do
        if self:IsIgnore(path) then
            goto continue
        end
        local excelFile = ExcelFile.New()
        if not excelFile:Open(path) then
            log.Error("打开文件[%s]失败", path)
        else
            local name = fs.GetFileName(path)
            self:OnReadFile(excelFile, name)
        end
        :: continue ::
    end
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
    local name = sheet:get_name()
    local last_row = sheet:get_last_row()
    local last_col = sheet:get_last_col()

    for y = self.Line.ValueStartCol, last_col do
        local cell = sheet:get_cell(self.Line.FieldNameLine, y)
        if not cell then
            error(string.format("文件:%s 分页:%s %s行不存在", fileName, name, y))
        end
        local value = cell:get_value()
        if not value and #value <= 0 then
            error(string.format("文件:%s 分页:%s %s行数据不合法", fileName, name, y))
        end
        fields[y] = value
    end

    for y = self.Line.ValueStartCol, last_col do
        local cell = sheet:get_cell(self.Line.ValueTypeLine, y)
        if not cell then
            error(string.format("文件:%s 分页:%s %s行不存在", fileName, name, y))
        end
        local value = cell:get_value()
        if not value and #value <= 0 then
            error(string.format("文件:%s 分页:%s %s行数据不合法", fileName, name, y))
        end
        types[y] = value
    end
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
                    if filed == nil then
                        log.Error("[%s=>%s] 字段名(%s) 不存在", fileName, name, y)
                        return false
                    end
                    item[filed] = self:DecodeByType(type, value)
                end
            end
        end
        if next(item) then
            table.insert(documents, item)
        end
    end
    for _, output in ipairs(self.output) do
        local handler = self.Handler[output.type]
        if handler == nil then
            log.Error("找不到导出处理函数:%s", output.type)
            return
        end
        local content = handler.Run(documents, types, fields, name)
        if content == nil or content == "" then
            return false
        end
        local dir = output.path
        local path = string.format("%s/%s%s", dir, name, output.ext)
        local file = io.open(path, "w")
        if file == nil then
            log.Error("打开文件[%s]失败", path)
            return false
        end
        file:write(content)
        file:close()
        log.Debug("导出文件%s到(%s)成功", name, path)
    end

    return true
end

function Excel:OnReadFile(excelFile, fileName)
    local sheets = excelFile:GetSheets()
    for _, sheet in ipairs(sheets) do
        local name = sheet:get_name()
        if self:IsIgnore(name) then
            goto continue
        end
        if not self:IsValid(name) then
            goto continue
        end
        if self:OnReadSheet(sheet, fileName) then
            log.Debug("导出文件分页 [%s=>%s] 成功", fileName, name)
        else
            log.Error("导出文件分页 [%s=>%s] 失败", fileName, name)
        end
        :: continue ::
    end
end

return Excel