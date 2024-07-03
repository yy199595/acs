<template>
    <div>
        <h3>城市列表</h3>
        <el-form inline="inline">
            <el-form-item label="城市">
                <el-input v-model="query.name"></el-input>
            </el-form-item>
            <el-form-item>
                <el-button type='primary' @click='on_btn_query'>查询</el-button>
            </el-form-item>
        </el-form>
        <el-table :data="from.list" border>
            <el-table-column label="编码" prop="code"></el-table-column>
            <el-table-column label="市" prop="name"></el-table-column>
            <el-table-column label="状态" >
                <template #default="scope">
                    <el-switch v-model="scope.row.open" @change="on_btn_open(scope.row)"></el-switch>
                </template>
            </el-table-column>
        </el-table>

        <div class="pagination">
            <el-pagination
                background
                layout="total, prev, pager, next"
                :current-page="query.page"
                :page-size="query.count"
                :total="query.length"
                @current-change="handlePageChange"
            ></el-pagination>
        </div>
    </div>

</template>

<script>
import {open_city, query_city_list} from "../api/city";
import {ElMessage, ElMessageBox} from "element-plus";

export default {
    name: "City",
    data() {
        return {
            from : {
                city : 5001,
                open : false,
                all : [],
                list : [],
                province : [],
            },
            query : {
                name : "",
                page : 1,
                count : 10,
                length : 0
            }
        }
    },
    methods : {
        on_btn_query() {
            this.from.list = []
            if(this.query.name === "") {
                const start = (this.query.page - 1) * 10
                for (let i = start; i < start + 10; i++) {
                    const item = this.from.all[i]
                    this.from.list.push({
                        code : item.code,
                        open : item.open,
                        name : item.parent_name + item.name
                    })
                }
                this.query.length = this.from.all.length
            }
            else {
                this.query.length = 0
                for (const item of this.from.all) {
                    const name = item.parent_name + item.name
                    if(name.indexOf(this.query.name) !== -1) {
                        this.from.list.push({
                            code : item.code,
                            open : item.open,
                            name : name
                        })
                        this.query.length++;
                        if(this.from.list.length >= 10)
                        {
                            return
                        }
                    }
                }
            }
        },
        handlePageChange(page) {
            this.query.page = page
            this.on_btn_query()
        },
        async on_btn_open(row) {
            const title = row.open ? `确定开放【${row.name}】吗?` :
                `确定关闭【${row.name}】吗`
            try {
                await ElMessageBox.confirm(title, '提示', {
                    type: 'error'
                });
            }
            catch (e) {
                row.open = !row.open
            }

            open_city(row.code).then(response=> {
                if(response.code === 0)
                {
                    ElMessage.success("开启成功")
                }
            })
        }
    },
    mounted() {
        query_city_list().then(response => {
            const result = response.data
            this.from.all = result.data
            this.from.all.sort((item1, item2)=> {
                if (item1.open && !item2.open) {
                    return -1;
                }
                if (!item1.open && item2.open) {
                    return 1;
                }
                return item1.code - item2.code
            })
            for (let i = 0; i < this.from.all.length; i++) {
                if(this.from.all[i].name === "市辖区")
                {
                    this.from.all[i].name = ""
                }
            }
            this.query.length = this.from.all.length
            this.on_btn_query()
        })
    }
}
</script>

<style scoped>

</style>
