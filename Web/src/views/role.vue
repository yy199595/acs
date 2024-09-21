<template>
    <el-form inline="inline">
        <el-form-item label="用户ID" style="width: 250px">
            <el-input v-model="input.user_id" placeholder="输入用户ID"></el-input>
        </el-form-item>
        <el-form-item>
            <el-button type="primary" @click="on_btn_query">查询</el-button>
        </el-form-item>
        <el-form-item>
            <el-button type='primary' @click='on_btn_edit'>保存</el-button>
        </el-form-item>
    </el-form>

    <div v-if="user_info.data">

        <el-descriptions title="基本信息" border :column="2">

            <el-descriptions-item label="UserId">{{ user_info.data.user_id }}</el-descriptions-item>
            <el-descriptions-item label="OpenID">{{ user_info.data._id }}</el-descriptions-item>
            <el-descriptions-item label="昵称">
                <el-input v-model="user_info.data.nick"></el-input>
            </el-descriptions-item>

            <el-descriptions-item label="性别">
                <el-select v-model="user_info.data.sex">
                    <el-option v-for="item in sex_options" :key="item.value" :value="item.value" :label="item.label"></el-option>
                </el-select>
            </el-descriptions-item>
            <el-descriptions-item label="身份">
                <el-select v-model="user_info.data.permission">
                    <el-option v-for="item in options" :key="item.value" :value="item.value" :label="item.label"></el-option>
                </el-select>
            </el-descriptions-item>
            <el-descriptions-item label="城市">{{ user_info.data.city_name }}</el-descriptions-item>
            <el-descriptions-item label="简介">{{ user_info.data.desc }}</el-descriptions-item>
            <el-descriptions-item label="年龄">{{ user_info.data.age }}</el-descriptions-item>
            <el-descriptions-item label="余额">{{ "¥" + ((user_info.data.amount || 0) / 100).toFixed(2) }}</el-descriptions-item>
            <el-descriptions-item label="会员类型" v-if="user_info.data.card_id === 0">{{ "非会员" }} </el-descriptions-item>
            <el-descriptions-item label="会员类型" v-if="user_info.data.card_id === 1">{{ "年卡会员"}} </el-descriptions-item>
            <el-descriptions-item label="会员类型" v-if="user_info.data.card_id === 2">{{ "永久会员"}} </el-descriptions-item>
            <el-descriptions-item label="会员类型" v-if="user_info.data.card_id === 3">{{ "普通群"}} </el-descriptions-item>

            <el-descriptions-item label="头像">
<!--                <el-link type="primary" :href="user_info.data.icon" target="_blank">{{ user_info.data.icon }}</el-link>-->
                <el-avatar size="large" shape="square" :src="user_info.data.icon"></el-avatar>
            </el-descriptions-item>
        </el-descriptions>
        <el-collapse accordion>
            <el-collapse-item title="活动列表" v-if="Array.isArray(user_info.data.activity_list)">
                <el-table :data="user_info.data.activity_list" border>
                    <el-table-column label="活动ID" prop="_id"></el-table-column>
                    <el-table-column label="标题" prop="title" show-overflow-tooltip></el-table-column>
                    <el-table-column label="地址" prop="address" show-overflow-tooltip></el-table-column>
                    <el-table-column label="状态" prop="status" :formatter="format_status"></el-table-column>
                    <el-table-column label="开始时间" prop="start_time" :formatter="format_time"></el-table-column>
                    <el-table-column label="结束时间" prop="stop_time" :formatter="format_time"></el-table-column>

                </el-table>
            </el-collapse-item>
            <el-collapse-item title="订单列表" v-if="Array.isArray(user_info.data.order_list)">
                <el-table :data="user_info.data.order_list" border>
                    <el-table-column label="订单ID" prop="_id"></el-table-column>
                    <el-table-column label="订单类型" prop="type" :formatter="format_order_type"></el-table-column>
                    <el-table-column label="订单状态" prop="status" :formatter="format_order_status"></el-table-column>
                    <el-table-column label="金额" prop="price"></el-table-column>
                    <el-table-column label="产品ID" prop="product_id"></el-table-column>
                    <el-table-column label="描述" prop="desc" show-overflow-tooltip></el-table-column>
                    <el-table-column label="创建时间" prop="create_time" :formatter="format_time"></el-table-column>
                </el-table>
            </el-collapse-item>
        </el-collapse>
    </div>

</template>

<script>
import {format_permiss, format_time} from "../api/format";
import {change_role_info, request_account_info} from "../api/account";
import {ElMessage, ElMessageBox} from "element-plus";

export default {
    name: "role",
    data() {
        return {
            input: {
                user_id: "",
            },
            user_info: {
                data: null
            },
            sex_options : [
                {
                    value : 1,
                    label : "男"
                },
                {
                    value: 0,
                    label: "女"
                }
            ],
            options : [
                {
                    value : 1,
                    label : "普通用户"
                },
                {
                    value : 3,
                    label : "领队"
                },
                {
                    value : 8,
                    label: "主创"
                },
                {
                    value : 9,
                    label: "商家"
                },
                {
                    value : 10,
                    label: "主理人"
                },
                {
                    value : 100,
                    label: "管理员"
                }
            ],
            format_time: format_time,
            format_permiss: format_permiss,

        }
    },

    mounted() {
        const userId = localStorage.getItem("role_id")
        if(userId ) {
            this.input.user_id = userId
            localStorage.removeItem("role_id")
            this.on_btn_query()
        }
    },
    methods: {

        async on_btn_edit() {
            await ElMessageBox.confirm('确定修改用户信息吗？', '提示', {
                type: 'error'
            });
            const item = {
                user_id : this.user_info.data.user_id,
                nick : this.user_info.data.nick,
                sex : this.user_info.data.sex,
                permission : this.user_info.data.permission
            }
            const response = await change_role_info(item)
            if(response.data.code === 0) {
                ElMessage.success("修改用户信息成功")
                this.on_btn_query()
            } else {
                ElMessage.error(response.data.error)
            }
        },

        on_btn_query() {
            request_account_info(this.input.user_id).then(response => {
                console.log(response)
                const result = response.data
                if (result.code !== 0) {
                    ElMessage.error(result.error)
                    return
                }
                this.user_info.data = result.data
                this.user_info.data.age = 0
                if (this.user_info.data.birthday > 0) {
                    const tmp = new Date(result.data.birthday * 1000).getFullYear()
                    this.user_info.data.age = new Date().getFullYear() - tmp
                }
            })
        },
        format_order_type(row, col, value) {
            switch (value) {
                case 1:
                    return "VIP订单"
                case 2:
                    return "活动订单"
            }
            return "未知类型"
        },
        format_status(row, column, cellValue) {
            switch (cellValue) {
                case 0:
                    return "报名中"
                case 1:
                    return "进行中"
                case 2:
                    return "已完成"
                case 3:
                    return "已解散"
            }
            return "未知"
        },
        format_order_status(row, col, value) {
            switch (value) {
                case 1: {
                    return "等待支付"
                }
                case 2:
                    return "支付完成"
                case 3:
                    return "等待退款"
                case 4:
                    return "已经退款"
                case 5:
                    return "已经过期"
                case 6:
                    return "确认收货"
            }
            return "未知类型"
        },
    }
}
</script>

<style scoped>

</style>
