<template>
    <el-form inline="inline">
        <el-form-item label="搜索">
            <el-input></el-input>
        </el-form-item>
        <el-form-item>
            <el-button type="primary">查询</el-button>
        </el-form-item>
        <el-form-item>
            <el-button type="primary" @click="on_btn_add">新增</el-button>
        </el-form-item>
    </el-form>

    <el-table :data="from.list" border>
        <el-table-column label="账号" prop="_id"></el-table-column>
        <el-table-column label="昵称" prop="name"></el-table-column>
        <el-table-column label="地区" prop="city_name"></el-table-column>
        <el-table-column label="权限" prop="permission" :formatter="format_permiss"></el-table-column>
        <el-table-column label="登录时间" prop="login_time" :formatter="format_time"></el-table-column>
        <el-table-column label="创建时间" prop="create_time" :formatter="format_time"></el-table-column>
        <el-table-column label="登录ip" prop="login_ip"></el-table-column>
        <el-table-column label="操作">
            <template #default="spoce">
                <el-button :disabled="is_admin() === false" type="danger">删除</el-button>
            </template>
        </el-table-column>
    </el-table>

    <el-dialog title="新增商户" v-model="from.show">
        <el-form :model="from.input" :rules="rules">
            <el-form-item label="地区">
                <el-select v-model="from.input.city"
                           placeholder="选择市">
                    <el-option v-for="item in from.city_list"
                               :key="item.code" :label="item.parent_name + item.name" :value="item.code"></el-option>
                </el-select>
            </el-form-item>
            <el-form-item label="昵称" prop="name">
                <el-input v-model="from.input.name" placeholder="输入账号"></el-input>
            </el-form-item>
            <el-form-item label="账号" prop="account">
                <el-input v-model="from.input.account" placeholder="输入账号"></el-input>
            </el-form-item>
            <el-form-item label="密码" prop="passwd">
                <el-input v-model="from.input.passwd" placeholder="输入登录密码"></el-input>
            </el-form-item>
            <el-form-item>
                <el-button type="primary" @click="on_btn_save">添加</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>
</template>

<script>
import {RequestAdminList, RequestRegister} from "../api/login";
import {ElMessage} from "element-plus";
import {format_permiss, format_time} from "../api/format";
import {request_city_list} from "../api/activity";
import {is_admin} from "../api/token";

export default {
    name: "mch_list",
    data() {
        return {
            from : {
                list : [],
                page : {
                    index : 1,
                    total : 0
                },
                input : {
                    name : "",
                    passwd : "",
                    city : 5001,
                    account : "",
                },
                show : false,
                city_list : []
            },
            rules : {
                account: [
                    {required: true, message: "请输入账号", trigger: "blur"},
                    {min: 3, max: 15, message: "账号长度在3到15个字左右", trigger: "blur"}
                ],
                name: [
                    {required: true, message: "请输入昵称", trigger: "blur"}
                ],
                passwd: [
                    {required: true, message: "请输入密码", trigger: "blur"},
                    {min: 8, max: 20, message: "账号长度在8到20个字左右", trigger: "blur"}
                ],
            },
            is_admin : is_admin,
            format_time : format_time,
            format_permiss : format_permiss
        }
    },
    async mounted() {
        const response = await RequestAdminList(this.from.page.index)
        if(response.data.code !== 0)
        {
            ElMessage.error("拉取商户列表失败")
            return
        }
        const result = response.data
        this.from.list = result.data.list
        this.from.page.total = result.data.count
    },
    methods : {
        async on_btn_add() {
            const response = await request_city_list()
            if(response.data.code !== 0)
            {
                ElMessage.error("加载城市列表失败")
                return
            }
            this.from.city_list = response.data.data
            this.from.show = true
        },
        async on_btn_save() {
            const response = await RequestRegister(this.from.input)
            if(response.data.code === 0)
            {
                ElMessage.success("添加商户成功")
                return
            }
            ElMessage.success("添加商户失败")
        }
    }
}
</script>

<style scoped>

</style>
