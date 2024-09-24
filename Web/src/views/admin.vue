<template>

    <el-form inline="inline">
        <el-form-item>
            <el-button type="primary" @click="newInfo.show=true" :disabled="userInfo.permission !== 100">新增</el-button>
        </el-form-item>
        <el-form-item>
            <el-button type="primary" @click="refresh_account_list">刷新</el-button>
        </el-form-item>
    </el-form>

    <el-table border :data="account_list">
        <el-table-column label="ID" prop="user_id" width="50%"></el-table-column>
        <el-table-column label="账号" prop="_id"></el-table-column>
        <el-table-column label="昵称" prop="name"></el-table-column>
        <el-table-column label="权限" prop="permission" :formatter="format_permiss"></el-table-column>
        <el-table-column label="注册时间" prop="create_time" :formatter="format_time"></el-table-column>
        <el-table-column label="上次登录时间" prop="login_time" :formatter="format_time"></el-table-column>
        <el-table-column label="上次登录IP" prop="login_ip"></el-table-column>
        <el-table-column label="操作">
            <template #default="scope">
                <el-button type="primary" :disabled="userInfo.user_id !== scope.row.user_id"
                           @click="on_btn_edit(scope.row)">修改
                </el-button>
                <el-button type="danger" :disabled="userInfo.permission !== 100" @click="on_btn_del(scope.row)">删除
                </el-button>
            </template>
        </el-table-column>
    </el-table>

    <div class="pagination">
        <el-pagination
            background
            layout="total, prev, pager, next"
            :current-page="query_info.page"
            :page-size="query_info.size"
            :total="query_info.total"
            @current-change="handlePageChange"
        ></el-pagination>
    </div>

    <el-dialog v-model="editInfo.show" title="修改账号信息">
        <el-descriptions border column="1">
            <el-descriptions-item label="昵称">
                <el-input v-model="editInfo.name"></el-input>
            </el-descriptions-item>
            <el-descriptions-item label="输入密码">
                <el-input v-model="editInfo.passwd" type="password" placeholder="请输入设置的密码"></el-input>
            </el-descriptions-item>
            <el-descriptions-item label="确认密码">
                <el-input v-model="editInfo.passwd1" type="password" placeholder="请再输入一次"></el-input>
            </el-descriptions-item>
        </el-descriptions>
        <el-form inline="inline" style="margin-top: 10px">
            <el-form-item>
                <el-button type="primary" @click="on_btn_change">确认</el-button>
            </el-form-item>
            <el-form-item>
                <el-button @click="editInfo.show=false">取消</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>

    <el-dialog v-model="newInfo.show" title="添加新用户">
        <el-descriptions border column="1">
            <el-descriptions-item label="昵称">
                <el-input placeholder="输入昵称" v-model="newInfo.name"></el-input>
            </el-descriptions-item>
            <el-descriptions-item label="账号">
                <el-input placeholder="输入账号(只能是数字或者字母)" v-model="newInfo.account"></el-input>
            </el-descriptions-item>
            <el-descriptions-item label="密码">
                <el-input placeholder="输入密码" v-model="newInfo.passwd"></el-input>
            </el-descriptions-item>
        </el-descriptions>
        <el-form inline="inline" style="margin-top: 10px">
            <el-form-item>
                <el-button type="primary" @click="on_btn_add">确定</el-button>
                <el-button @click="newInfo.show = false">取消</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>
</template>

<script>
import {format_permiss, format_time} from "../api/format";

import {httpRequest} from "../utils/httpRequest"
import {ElMessage, ElMessageBox} from "element-plus";
import {useRouter} from "vue-router";
import {remove_token} from "../api/token";

export default {
    name: "admin",
    data() {
        return {
            query_info: {
                page: 1,
                size: 10,
                total: 0
            },
            userInfo: {},
            account_list: [],
            newInfo: {
                show: false,
                name: "",
                account: "",
                passwd: "",
                permiss: 1,
            },
            editInfo: {
                show: false,
                name: "",
                passwd: "",
                passwd1 : ""
            }
        }
    },

    methods: {
        format_permiss,
        format_time,

        async on_btn_change() {

            let count = 0
            const request = {}
            if (this.editInfo.passwd.length > 0) {
                if (this.editInfo.passwd !== this.editInfo.passwd1) {
                    ElMessage.error("两次输入的密码不一致")
                    return
                }
                count++;
                request.passwd = this.editInfo.passwd
            }
            if (this.editInfo.name.length > 0) {
                count++;
                request.name = this.editInfo.name
            }
            if (count === 0) {
                this.editInfo.show = false
                return
            }
            await ElMessageBox.confirm('确定修改账号信息吗？', '提示', {
                type: 'error'
            });
            const response = await httpRequest.POST("/admin/update", request)
            this.editInfo.show = false
            if(response.data.code === 0) {
                remove_token()
                ElMessage.success("修改账号信息成功")
            } else {
                ElMessage.error(response.data.error)
            }
        },

        async on_btn_del(data) {

            await ElMessageBox.confirm('确定删除账号吗？', '提示', {
                type: 'error'
            });
            const response = await httpRequest.GET("/admin/remove?id=" + data.user_id)
            if(response.data.code === 0) {
                await this.refresh_account_list();
                ElMessage.success("删除账号成功")
                return
            }
            ElMessage.error("删除账号失败")
        },

        on_btn_edit(data) {
            this.editInfo.name = data.name
            this.editInfo.passwd = ""
            this.editInfo.show = true
        },
        async refresh_account_list() {
            this.account_list = []
            this.query_info.total = 0
            const response = await httpRequest.GET("/admin/list?page=" + this.query_info.page)
            console.log(JSON.stringify(response.data.list))
            if (response.data.code === 0) {
                this.account_list = response.data.list
                this.query_info.total = response.data.count
            } else {
                ElMessage.error(response.data.error)
            }
        },
        async on_btn_add() {
            if (this.newInfo.name.length <= 0) {
                ElMessage.error("请输入昵称")
                return
            }
            if (this.newInfo.account.length <= 0) {
                ElMessage.error("请输入账号")
                return
            }
            if (this.newInfo.passwd.length <= 0) {
                ElMessage.error("请输入密码")
                return
            }
            this.newInfo.permiss = 20
            const response = await httpRequest.POST("/admin/register", this.newInfo)
            this.newInfo.show = false
            if (response.data.code === 0) {
                await this.refresh_account_list();
                ElMessage.success("新增账号成功")
                return
            }
            ElMessage.error(response.data.error)
        },
        async handlePageChange(page) {
            this.query_info.page = page
            await this.refresh_account_list()
        }
    },

    async mounted() {

        const user_json = localStorage.getItem("user_info")
        if (user_json && user_json.length > 0) {
            this.userInfo = JSON.parse(user_json)
        } else {

        }
        console.log(JSON.stringify(this.userInfo))
        await this.handlePageChange(1)
    },


}
</script>


<style scoped>

</style>