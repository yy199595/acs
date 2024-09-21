<template>
    <h3>账号列表</h3>
    <el-form inline="inline">
        <el-form-item label="选择集合" v-if="input.table_list.length > 0" style="width: 100rpx">
            <el-cascader v-model="input.tab" :options="input.table_list" placeholder="选择集合"
                         @change="on_change_collect"></el-cascader>
        </el-form-item>

        <el-form-item label="排序">
            <el-cascader :options="input.sorter" v-model="input.sort" placeholder="排序方式" @change="on_sorter_change"></el-cascader>
        </el-form-item>


        <el-form-item label="条件">
            <el-select style='width: 250px' v-model="input.field" placeholder="选择字段">
                <el-option v-for="item in input.fields" :key="item" :label="item"
                           :value="item"/>
            </el-select>
        </el-form-item>

        <el-form-item label="=">
            <el-input v-model="input.value"></el-input>
        </el-form-item>

        <el-form-item>
            <el-button type="primary" @click="on_btn_query">查询</el-button>
        </el-form-item>

    </el-form>

    <el-dialog v-model="from.show" title="查看集合" destroy-on-close>
        <json-viewer :data="from.data"></json-viewer>

        <el-form style="margin-top: 10px">
            <el-form-item>
                <el-button type="success" @click="on_edit_data()" disabled>修改</el-button>
                <el-button @click="from.show = false">取消</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>

    <el-table :data="input.list" border style="width: 100%">
        <el-table-column v-for="key in input.fields" :prop="key"
                         :label="key" show-overflow-tooltip :formatter="format_text"></el-table-column>
        <el-table-column label="操作">
            <template #default="scope">
                <el-button type="primary" @click="on_btn_look(scope.row)">查看</el-button>
            </template>
        </el-table-column>
    </el-table>

    <div class="pagination">
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
import {delete_table, query_table} from "../api/city";
import {ElMessage, ElMessageBox} from "element-plus";
import {httpRequest} from "../utils/httpRequest"
import JsonViewer from "./json_view.vue";

export default {
    name: "mongodb",
    components: {JsonViewer},
    data() {
        return {
            input : {
                tab : [],
                field : "",
                value : "",
                page : 1,
                total : 0,
                list : [],
                fields : [],
                sorter : [],
                sort : [],
                field_name : "",
                table_list : [],
            },
            from : {
                data : {},
                line : 0,
                show : false,
                html : ""
            }
        }
    },
    methods : {

        on_change_collect() {
            this.input.field = ""
            this.input.value = ""
            this.input.page = 1
            this.on_btn_query()
        },

        on_sorter_change(res) {
            this.input.page = 1
            this.on_btn_query()
            console.log(JSON.stringify(res))
        },
        async on_edit_data() {
            const tab = this.input.tab.join(".")
            const updater = JSON.parse(this.from.data)
            const id = updater._id
            updater["_id"] = undefined
            const response = await httpRequest.POST("/table_mgr/update", {
                tab : tab,
                _id : id,
                updater : updater
            })
            if(response.data.code === 0) {
                ElMessage.success("修改成功")
            } else {
                ElMessage.error(response.data.error);
            }
            this.from.show = false
        },

        on_btn_query() {
            this.input.fields = []

            const data = {
                filter : {},
                tab : this.input.tab.join("."),
                page : this.input.page
            }

            if(this.input.sort.length === 2)
            {
                data.sorter = { }
                const key = this.input.sort[0]
                data.sorter[key] = this.input.sort[1]
            }

            if(this.input.field.length > 0 && this.input.value.length > 0)
            {
                data.filter[this.input.field] = {
                    "$regex" : this.input.value,
                    "$options" : "i"
                }
                try {
                    const num = parseInt(this.input.value)
                    if(num)
                    {
                        data.filter[this.input.field] = num
                    }
                }
                catch (e) {

                }
            }

            query_table(data).then(response=> {

                this.input.list = []
                this.input.total = 0
                if(response.data.code === 0 && Array.isArray(response.data.data.list)) {
                    this.input.fields.push("_id")
                    if(this.input.sort.length === 2)
                    {
                        this.input.fields.push(this.input.sort[0])
                    }
                    if(this.input.field.length > 0) {
                        this.input.fields.push(this.input.field)
                    }
                    this.input.sorter = []
                    this.input.list = response.data.data.list
                    this.input.total = response.data.data.count
                    for (let i = 0; i < this.input.list.length; i++) {
                        const item = this.input.list[i]
                        for (const itemKey in item) {
                            const value = item[itemKey]
                            const index = this.input.fields.findIndex(item => item === itemKey)
                            if (index === -1) {
                                if(typeof(value) === "number") {
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
                                    this.input.fields.push(itemKey)
                                } else if(typeof(value) === "string") {
                                    this.input.fields.push(itemKey)
                                }
                            }
                        }
                    }
                    const index = this.input.table_list.findIndex(item=> item === this.input.tab)
                    if(index === -1)
                    {
                        this.input.table_list.push(this.input.tab)
                        localStorage.setItem("table_list", JSON.stringify(this.input.table_list))
                    }
                }
            })
        },
        on_btn_look(data) {
            this.from.show = true
            this.from.line = 2
            this.from.data = data
            // for (let i = 0; i < this.from.data.length; i++) {
            //     if(this.from.data[i] === '\n') {
            //         this.from.line++
            //     }
            // }
        },
        async on_btn_delete() {
            await ElMessageBox.confirm('确定删除集合吗吗？', '提示', {
                type: 'error'
            });
            const name = this.input.tab
            delete_table(name).then(response=> {
                if(response.data.code === 0)
                {
                    ElMessage.success("删除集合成功")
                    const index = this.input.table_list.findIndex(item => item === name)
                    if (index !== -1) {
                        this.input.table_list.slice(index, 1)
                    }
                    localStorage.setItem("table_list", JSON.stringify(this.input.table_list))
                }
            })
        },
        format_text(row, column, cellValue) {
            const type_name = typeof cellValue
            if(type_name === "number" && column.label.indexOf("time") > -1)
            {
                const timestamp = new Date(cellValue * 1000); // 将时间戳转换为Date对象
                return timestamp.toLocaleString();
            }

            if (type_name === "object" || Array.isArray(cellValue)) {
                return JSON.stringify(cellValue)
            } else if (type_name === "boolean") {
                return cellValue ? "true" : "false"
            } else if(type_name === "number") {
                return cellValue
            }
            else if(!cellValue) {
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
        if(response.data.code === 0)
        {
            this.input.table_list = response.data.data
        }
        console.log(JSON.stringify(this.input.table_list))
    }
}
</script>

<style scoped>

</style>