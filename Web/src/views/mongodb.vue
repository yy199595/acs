<template>
    <h3>MongoDB</h3>
    <el-form inline="inline">
        <el-form-item label="选择集合" v-if="input.table_list.length > 0" style="width: 100rpx">
            <el-cascader v-model="input.tab" :options="input.table_list" placeholder="选择集合"
                         @change="on_change_collect"></el-cascader>
        </el-form-item>

        <el-form-item label="排序">
            <el-cascader :options="input.sorter" v-model="input.sort" placeholder="排序方式"
                         @change="on_sorter_change"></el-cascader>
        </el-form-item>

        <el-form-item label="条件">
            <el-cascader :options="input.query_fields" v-model="input.query_field"></el-cascader>
        </el-form-item>

        <el-form-item v-if="input.query_field.length > 0">
            <el-input v-model="input.value"></el-input>
        </el-form-item>

        <el-form-item>
            <el-button type="primary" plain @click="on_btn_query()">查询</el-button>
            <el-button type="danger" plain @click="on_btn_remove()">删除</el-button>
            <el-button type="info" plain @click="on_btn_download()">下载</el-button>
<!--            <el-button type="info" plain @click="on_btn_backup()">备份</el-button>-->
        </el-form-item>

    </el-form>

    <el-dialog v-model="from.show" title="查看集合" destroy-on-close>
        <json-viewer :data="from.data" :editable="from.edit" @update:data="on_input_data"></json-viewer>

        <el-form style="margin-top: 10px" v-show="from.edit">
            <el-form-item>
                <el-button :disabled="from.disable" type="success" @click="on_edit_data()">修改</el-button>
                <el-button @click="from.show = false">取消</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>

    <el-dialog v-model="download.show" title="下载文件">
        <el-form>
            <el-form-item>
                <el-progress type="circle"
                             :percentage="((download.down_count / download.count) * 100).toFixed(2)"></el-progress>
            </el-form-item>
            <el-form-item>{{ `正在下载文件[${download.file_name}] ${download.down_count}/${download.count}` }}}
            </el-form-item>
        </el-form>
    </el-dialog>

    <el-table v-if="input.list.length > 0" :data="input.list" border v-loading="from.loading">
        <el-table-column v-for="key in input.fields" :prop="key" align="center" min-width="120"
                         :label="key" show-overflow-tooltip :formatter="format_text"></el-table-column>

        <el-table-column label="操作" width="240" style="flex-direction: row" fixed="right" align="center">
            <template #default="scope">
                <el-button type="primary" plain icon="view" @click="on_btn_look(scope.row, false)"></el-button>
                <el-button type="warning" plain icon="edit" @click="on_btn_look(scope.row, true)"></el-button>
                <el-button type="danger" plain icon="delete" @click="on_btn_delete(scope.row)"></el-button>
            </template>
        </el-table-column>
    </el-table>

    <div class="pagination" v-if="input.list.length > 0">
        <el-pagination
            background
            layout="total, prev, pager, next"
            :current-page="input.page"
            :page-size="10"
            :total="input.total"
            @current-change="handlePageChange"
        ></el-pagination>
    </div>
</template>

<script>
import {query_table} from "../api/city";
import {ElMessage, ElMessageBox} from "element-plus";
import {httpRequest} from "../utils/httpRequest"
import JsonViewer from "./json_view.vue";

export default {
    name: "mongodb",
    components: {JsonViewer},
    data() {
        return {
            input: {
                tab: [],
                field: "",
                value: "",
                page: 1,
                total: 0,
                list: [],
                fields: [],
                fields_types: {},
                sorter: [],
                sort: [],
                field_name: [],
                table_list: [],
                regex: false,
                query_field: "",
                query_fields: []
            },
            from: {
                data: {},
                line: 0,
                show: false,
                html: "",
                edit: false,
                text: "",
                disable: true,
                loading: false
            },
            download: {
                count: 0,
                show: false,
                file_name: "",
                down_count: 0
            },
            query_opts: {
                ["="]: {label: "等于", value: ""},
                [">"]: {label: "大于", value: "$gt"},
                ["<"]: {label: "小于", value: "$$lt"},
                ["!="]: {label: "不等于", value: "$ne"},
                [">="]: {label: "大于等于", value: "$gte"},
                ["<="]: {label: "小于等于", value: "$lte"},
                ["regex"]: {label: "正则匹配", value: "$regex"},
            }
        }
    },
    methods: {

        on_change_collect() {
            this.input.field = ""
            this.input.value = ""
            this.input.fields = []
            this.input.query_fields = []
            this.input.page = 1
            this.on_btn_query()
        },

        on_select_field(res) {
            this.input.regex = this.input.fields_types[res] === "string"
        },

        on_input_data(str) {
            this.from.text = str
            this.from.disable = str === JSON.stringify(this.from.data, null, 2)
        },

        on_sorter_change(res) {
            this.input.page = 1
            this.on_btn_query()
            console.log(JSON.stringify(res))
        },
        async on_edit_data() {
            const tab = this.input.tab.join(".")
            try {
                const updater = JSON.parse(this.from.text)
                if (updater._id !== this.from.data._id) {
                    this.from.show = false
                    ElMessage.error("不能修改_id字段")
                    return
                }
                const response = await httpRequest.POST("/table_mgr/update", {
                    tab: tab,
                    data: updater
                })
                if (response.data.code === 0) {
                    ElMessage.success("修改成功")
                } else {
                    ElMessage.error(response.data.error);
                }
                this.from.show = false
            } catch (e) {
                ElMessage.error("修改内容不是json")
            }
        },

        get_filter() {
            const data = {
                filter: {},
                tab: this.input.tab.join("."),
                page: this.input.page
            }

            if (this.input.sort.length === 2) {
                data.sorter = {}
                const key = this.input.sort[0]
                data.sorter[key] = this.input.sort[1]
            }

            if (this.input.value.length > 0 && this.input.query_field.length === 2) {

                data.filter.value = this.input.value
                data.filter.key = this.input.query_field[0]
                data.filter.opt = this.input.query_field[1]
                data.filter.type = this.input.fields_types[data.filter.key]
            }
            return data
        },

        async on_btn_remove() {

            const filter = this.get_filter()
            const response = await httpRequest.POST("/table_mgr/count", filter)

            const count = response.data.data.count
            await ElMessageBox.confirm(`确定删除${count}条数据吗`, '提示', {
                type: 'error'
            });
        },

        createWriteStream(fileName) {
            // 创建一个新的 Blob 构建器
            const parts = [];

            return {
                write: (chunk) => {
                    parts.push(new Blob([chunk], { type: 'text/plain' }));
                },
                close: () => {
                    // 合并所有 Blob 部分
                    const fullBlob = new Blob(parts, { type: 'text/plain;charset=utf-8' });
                    return URL.createObjectURL(fullBlob);
                }
            };
        },

        writeChunk(stream, chunk) {
            return new Promise((resolve, reject) => {
                try {
                    stream.write(chunk);
                    stream.write('\n');
                    resolve();
                } catch (error) {
                    reject(error);
                }
            });
        },

        downloadFile(url, fileName) {
            const a = document.createElement('a');
            a.href = url;
            a.download = fileName;
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
            URL.revokeObjectURL(url);
        },

        closeStream(stream) {
            return new Promise((resolve, reject) => {
                try {
                    const url = stream.close();
                    resolve(url);
                } catch (error) {
                    reject(error);
                }
            });
        },

        async on_btn_download() {

            const filter = this.get_filter()
            let response = await httpRequest.POST("/table_mgr/count", filter)

            const count = response.data.data.count
            await ElMessageBox.confirm(`确定下载${count}条数据吗`, '提示', {
                type: 'info'
            });
            let page = 0
            let list = []
            this.download.show = true
            this.download.count = count
            this.download.down_count = 0
            this.download.file_name = `${filter.tab}.json`
            const stream = this.createWriteStream(this.download.file_name)
            do {
                page++
                filter.page = page
                response = await httpRequest.POST("/table_mgr/find_all", filter)
                if (response.data.code !== 0) {
                    ElMessage.error("下载失败")
                    break
                }
                list = response.data.data.list
                for (const document of list) {
                    await this.writeChunk(stream, JSON.stringify(document))
                }
                this.download.down_count += list.length
                await new Promise(resolve => setTimeout(resolve, 0));
            }
            while (this.download.down_count < this.download.count)

            const downloadUrl = await this.closeStream(stream);
            this.downloadFile(downloadUrl, this.download.file_name);
            ElMessage.success("下载成功")
        },

        async on_btn_backup() {
            const filter = this.get_filter()
            const response = await httpRequest.POST("/table_mgr/count", filter)

            const count = response.data.data.count
            await ElMessageBox.confirm(`确定备份${count}条数据吗`, '提示', {
                type: 'info'
            });
        },

        async on_btn_query() {

            this.from.loading = true
            const filter = this.get_filter()
            try {
                const response = await query_table(filter)

                this.input.list = []
                this.input.total = 0
                if (response.data.code === 0 && Array.isArray(response.data.data.list)) {
                    //this.input.fields.push("_id")
                    if (this.input.sort.length === 2) {
                        this.input.fields.push(this.input.sort[0])
                    }
                    if (this.input.field.length > 0) {
                        this.input.fields.push(this.input.field)
                    }

                    this.input.total = response.data.data.count
                    this.input.fields_types = response.data.data.types

                    this.input.fields = []
                    for (const responseKey in this.input.fields_types) {
                        this.input.fields.push(responseKey)
                    }
                    this.input.fields.sort()

                    const typeItems = {}
                    for (const key of this.input.fields) {
                        typeItems[key] = this.input.fields_types[key]
                    }
                    this.input.list.push(typeItems)

                    for (const itemKey of this.input.fields) {
                        switch (this.input.fields_types[itemKey]) {
                            case "int32":
                            case "int64": {
                                this.input.sorter.push({
                                    label: itemKey,
                                    value: itemKey,
                                    children: [{
                                        label: "升序",
                                        value: 1
                                    }, {
                                        label: "降序",
                                        value: -1
                                    }]
                                })
                                this.input.query_fields.push({
                                    label: itemKey,
                                    value: itemKey,
                                    children: [
                                        this.query_opts["="],
                                        this.query_opts[">"],
                                        this.query_opts["<"],
                                        this.query_opts["!="],
                                        this.query_opts[">="],
                                        this.query_opts["<="],
                                    ]
                                })
                                break
                            }
                            case "string": {
                                this.input.query_fields.push({
                                    label: itemKey,
                                    value: itemKey,
                                    children: [
                                        this.query_opts["="],
                                        this.query_opts[">"],
                                        this.query_opts["<"],
                                        this.query_opts["!="],
                                        this.query_opts[">="],
                                        this.query_opts["<="],
                                        this.query_opts["regex"],
                                    ]
                                })
                                break
                            }
                            default:

                                break
                        }
                    }

                    this.input.list.push(... response.data.data.list)

                    const index = this.input.table_list.findIndex(item => item === this.input.tab)
                    if (index === -1) {
                        this.input.table_list.push(this.input.tab)
                        localStorage.setItem("table_list", JSON.stringify(this.input.table_list))
                    }
                }
            } catch (e) {

            }
            this.from.loading = false

        },
        on_btn_look(data, edit) {
            this.from.text = ""
            this.from.show = true
            this.from.line = 2
            this.from.edit = edit
            this.from.data = data
            this.from.disable = true
        },
        async on_btn_delete(data) {
            await ElMessageBox.confirm('确定删除数据吗？', '提示', {
                type: 'error'
            });
        },

        format_text(row, column, cellValue) {
            const type_name = typeof cellValue
            if (type_name === "number") {
                if (column.label.indexOf("time") > -1) {
                    if (cellValue === 0) {
                        return cellValue.toString();
                    }
                    const timestamp = new Date(cellValue * 1000); // 将时间戳转换为Date对象
                    return timestamp.toLocaleString();
                } else if (column.label.indexOf("amount") > -1) {
                    return `¥${cellValue / 100.0}`
                }
            }

            if (type_name === "object" || Array.isArray(cellValue)) {
                return JSON.stringify(cellValue)
            } else if (type_name === "boolean") {
                return cellValue ? "true" : "false"
            } else if (type_name === "number") {
                return cellValue
            } else if (!cellValue) {
                return "unset"
            }
            return cellValue.toString()
        },
        handlePageChange(page) {
            this.input.page = page
            this.on_btn_query()
        }
    },
    async mounted() {
        const response = await httpRequest.GET("table_mgr/tables")
        if (response.data.code === 0) {
            this.input.table_list = response.data.data.list
            for (const firstItem of this.input.table_list) {
                if (firstItem.children.length > 0) {
                    this.input.tab.push(firstItem.label)
                    this.input.tab.push(firstItem.children[0].label)
                    this.on_change_collect()
                    break
                }
            }
        }
    }
}
</script>

<style scoped>

</style>