<template>
    <div>
        <div class="container">
            <el-form inline="inline">
                <el-form-item>
                    <el-select style='width: 250px' v-model="query.type" placeholder="选择类型">
                        <el-option v-for="item in HttpTypeArray" :key="item.type" :label="item.name"
                                   :value="item.type"/>
                    </el-select>
                </el-form-item>
                <el-form-item>
                    <el-form-item label='接口url'>
                        <el-input v-model='query.name'></el-input>
                    </el-form-item>
                    <el-form-item>
                        <el-button type='primary' @click='getData'>查询</el-button>
                    </el-form-item>
                </el-form-item>
            </el-form>

            <el-table :data="tableData" border class="table" ref="multipleTable" header-cell-class-name="table-header">
                <el-table-column prop="url" label="url" align="left"></el-table-column>
                <el-table-column prop="desc" label="描述" align="left"></el-table-column>
                <el-table-column prop="permiss" label="权限" align="left" :formatter="format_permiss"></el-table-column>
                <el-table-column prop="method" label="方法" align="left"></el-table-column>
                <el-table-column prop="content" label="参数类型" align="left"></el-table-column>
<!--                <el-table-column label="协程" align="center">-->
<!--                    <template #default="scope">-->
<!--                        <el-switch disabled v-model="scope.row.async"></el-switch>-->
<!--                    </template>-->
<!--                </el-table-column>-->

                <el-table-column label="操作" width="220" align="center">
                    <template #default="scope">
                        <el-button type="primary" @click="on_btn_look(scope.row)">查看</el-button>
                    </template>
                </el-table-column>
            </el-table>
            <div class="pagination">
                <el-pagination
                    background
                    layout="total, prev, pager, next"
                    :current-page="query.page"
                    :page-size="query.pageSize"
                    :total="query.pageTotal"
                    @current-change="handlePageChange"
                ></el-pagination>
            </div>
        </div>

        <!-- 编辑弹出框 -->
        <el-dialog title="编辑" v-model="look_info.show" width="30%">
            <el-form label-width="70px">
                <el-form-item label="环境" label-width="100px">
                    <el-input v-model="look_info.conf.host" @input="on_input_change"></el-input>
                </el-form-item>
                <el-form-item label="Token" label-width="100px">
                    <el-input v-model="look_info.conf.token"></el-input>
                </el-form-item>
                <el-form-item label="url" label-width="100px">
                    <el-input v-model="look_info.url" :disabled="true"></el-input>
                </el-form-item>
                <el-form-item label="Content-Type" label-width="100px" v-if="look_info.data.content.length > 0">
                    <el-input disabled v-model="look_info.data.content"></el-input>
                </el-form-item>

                <el-form-item label="请求" label-width="100px" v-if="look_info.data.request.length > 0">
                    <el-input v-model="look_info.data.request" type="textarea" :rows='10'></el-input>
                </el-form-item>
                <el-form-item label="耗时" label-width="100px" v-if="look_info.ms > 0">
                    {{look_info.ms + "ms"}}
                </el-form-item>
                <el-form-item label="响应" label-width="100px" v-if="look_info.response != null">
                   <el-input v-model="look_info.response" type="textarea" :rows="10"></el-input>
                </el-form-item>

            </el-form>
            <template #footer>
				<span class="dialog-footer">
					<el-button type="primary" @click="on_btn_invoke">调用</el-button>
                    <el-button @click="look_info.show = false">取消</el-button>
				</span>
            </template>
        </el-dialog>
    </div>
</template>

<script>
import {ElMessage} from 'element-plus';
import {RequestHttpList} from '../api/server';
import axios from "axios";



export default {
    name: "http",
    components : {

    },
    data() {
        return {
            query: {
                name: '',
                type: "",
                page: 1,
                pageSize: 10,
                pageTotal: 0,
            },
            toolbars : {
                help: true, // 帮助
            },
            HttpTypeArray: [
                {
                    type: "",
                    name: "全部接口"
                },
                {
                    type: "GET",
                    name: "GET接口"
                },
                {
                    type: "POST",
                    name: "POST接口"
                },
            ],
            tableData: [],
            look_info : {
                show : false,
                data : null,
                conf : {
                    host : "http://127.0.0.1:80",
                    token : ""
                },
                url : "",
                ms : 0,
                response : null
            }
        }
    },

    methods: {
        getData() {
            RequestHttpList(this.query).then(res => {
                this.tableData = res.data.list
                this.query.pageTotal = res.data.count
                this.tableData.map((data) => {
                    if (data.content === "" && data.request.length > 0) {
                        data.content = "application/x-www-form-urlencoded"
                    }
                })
            }).catch(() => {
                ElMessage.error("拉取数据失败")
            });
        },
        handlePageChange(val) {
            this.query.page = val;
            this.getData();
        },
        async on_btn_invoke() {
            const config = {
                url: this.look_info.url,
                method: this.look_info.data.method,
            }
            if (this.look_info.data.request.length > 0) {
                if (this.look_info.data.content.length > 0) {
                    config.headers = {
                        "Content-Type": this.look_info.data.content
                    }
                }
                const obj = JSON.parse(this.look_info.data.request)
                if (config.method === "GET") {
                    config.params = obj
                } else {
                    config.data = obj
                }
            }
            if (this.look_info.conf.token.length > 0) {
                config.headers = {
                    "Access-Token": this.look_info.conf.token
                }
            }
            try {
                const t1 = new Date().getTime()
                const response = await axios.create().request(config)
                if(response.status === 200)
                {
                    localStorage.setItem("api_conf", JSON.stringify(this.look_info.conf))
                }
                const t2 = new Date().getTime()
                this.look_info.ms = t2 - t1
                this.look_info.response = JSON.stringify({
                    code : response.status,
                    text : response.statusText,
                    headers : response.headers,
                    data : response.data
                }, null, 4)
            }
            catch (e) {
                this.look_info.response = JSON.stringify({
                    code : e.response.status,
                    text : e.response.statusText,
                    headers : e.response.headers,
                }, null, 4)
            }


        },
        on_btn_query() {

        },
        on_btn_look(row) {
            this.look_info.response = null
            const item = JSON.stringify(row)
            this.look_info.data = JSON.parse(item)
            if(this.look_info.data.request.length > 0)
            {
                const obj = JSON.parse(this.look_info.data.request)
                this.look_info.data.request = JSON.stringify(obj, null, 2)
            }
            const conf = localStorage.getItem("api_conf")
            if(conf)
            {
                this.look_info.conf = JSON.parse(conf)
            }
            this.look_info.show = true
            this.look_info.url =this.look_info.conf.host + this.look_info.data.url
        },
        on_input_change() {
            this.look_info.url = this.look_info.conf.host + this.look_info.data.url
            console.log(this.look_info.url)
        },
        format_permiss (row, column, cellValue){
            switch(cellValue)
            {
                case 1:
                    return "普通用户";
                case 2:
                    return "VIP";
                case 10:
                    return "领队";
                case 100:
                    return "管理员"
            }
            return "未知"
        }
    },
    mounted() {
        this.getData()
    }
}
</script>

<style scoped>

.table {
    width: 100%;
    font-size: 14px;
}

</style>
