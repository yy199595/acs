<template>
    <h3>账号列表</h3>
    <el-form inline="inline">
        <el-form-item label="选择集合" v-if="input.table_list.length > 0">
            <el-select style='width: 250px' v-model="input.tab" placeholder="选择集合">
                <el-option v-for="item in input.table_list" :key="item" :label="item"
                           :value="item"/>
            </el-select>
        </el-form-item>

        <el-form-item label="集合">
            <el-input v-model="input.tab"></el-input>
        </el-form-item>
        <el-form-item>
            <el-button type="primary" @click="on_btn_query">查询</el-button>
            <el-button type="danger" @click="on_btn_delete">删除</el-button>
        </el-form-item>
    </el-form>

    <el-dialog v-model="from.show" title="查看集合">
        <el-input v-model="from.data" type="textarea" :rows="from.line"></el-input>
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
import {format_time} from "../api/format";

export default {
    name: "mongodb",
    data() {
        return {
            input : {
                tab : "",
                page : 1,
                total : 0,
                list : [],
                fields : [],
                field_name : "",
                table_list : [],
            },
            from : {
                data : "",
                line : 0,
                show : false
            }
        }
    },
    methods : {
        on_btn_query() {
            this.input.fields = []
            this.input.fields.push("_id")
            query_table(this.input.tab, this.input.page).then(response=> {
                if(response.data.code === 0) {
                    this.input.list = response.data.data.list
                    this.input.total = response.data.data.count
                    console.log(this.input.list)
                    for (let i = 0; i < this.input.list.length; i++) {
                        const item = this.input.list[i]
                        for (const itemKey in item) {
                            const index = this.input.fields.findIndex(item => item === itemKey)
                            if (index === -1) {
                                this.input.fields.push(itemKey)
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
            this.from.line = this.input.fields.length + 2
            this.from.data = JSON.stringify(data, null, 4)
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
            console.log(column.label)
            if (type_name === "object" || Array.isArray(cellValue)) {
                return JSON.stringify(cellValue)
            } else if (type_name === "boolean") {
                return cellValue ? "true" : "false"
            } else if(!cellValue) {
                return "unset"
            }
            return cellValue.toString()
        },
        handlePageChange(page) {
            this.input.page = page
            this.on_btn_query()
        }
    },
    mounted() {
        const list = localStorage.getItem("table_list")
        if(list)
        {
            this.input.table_list = JSON.parse(list)
        }
    }
}
</script>

<style scoped>

</style>