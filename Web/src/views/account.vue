<template>
    <el-row style='padding-left:15px;padding-top:10px'>

        <el-form inline="inline">

            <el-form-item label="城市" style="width: 250px">
                <el-select v-model="city_info.city_id" clearable placeholder="请选择城市">
                    <el-option v-for="(item, index) in city_info.list" :key="index"
                               :label="item.label" :value="item.value"></el-option>
                </el-select>
            </el-form-item>

            <el-form-item label='用户ID' style="width: 250px">
                <el-input v-model='query_info.open_id' placeholder='输入用户ID'></el-input>
            </el-form-item>

            <el-form-item label='用户昵称' style="width: 250px">
                <el-input v-model='query_info.nick' placeholder='输入用户昵称'></el-input>
            </el-form-item>

            <el-form-item>
                <el-button type='primary' @click='on_btn_query'>查询</el-button>
            </el-form-item>
        </el-form>

        <el-table :data="account_list" border class="table" ref="multipleTable" header-cell-class-name="table-header">
            <el-table-column label="头像" width="100rpx">
                <template #default="scope">
                    <el-avatar size="large" shape="square" :src="scope.row.icon"></el-avatar>
                </template>
            </el-table-column>
            <el-table-column label="用户ID" width="100rpx">
                <template #default="scope">
                    <el-link type="primary" @click='on_btn_look(scope.row.user_id)'>{{ scope.row.user_id }}</el-link>
                </template>
            </el-table-column>
            <el-table-column prop="sex" label="性别" :formatter="format_sex" width="100rpx"></el-table-column>
            <el-table-column prop="nick" label="昵称" width="150rpx"></el-table-column>
            <el-table-column prop="city_name" label="城市" width="150rpx"></el-table-column>
            <!--            <el-table-column prop="login_type" label="登录类型" :formatter="format_login_type"></el-table-column>-->
            <el-table-column prop="create_time" label="创建时间" width="180rpx" :formatter="format_time"></el-table-column>
            <el-table-column prop="card_id" label="用户类型" :formatter="format_vip_type"></el-table-column>
            <el-table-column prop="desc" label="简介" show-overflow-tooltip></el-table-column>
            <el-table-column label="操作" width="200rpx" style="flex-direction: row">
                <template #default="scope">
                    <el-button type="danger" @click="on_btn_delete(scope.row._id)">删除</el-button>
                    <el-button type="warning" @click="on_btn_bannend(scope.row._id)">封禁</el-button>
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

        <el-dialog title="封禁账号" v-model="dialog_info.show" width="50%">
            <el-form inline="inline">
                <el-form-item label="账号">
                    <el-input disabled v-model="dialog_info.account"></el-input>
                </el-form-item>
                <el-form-item label="封禁时长">
                    <el-select v-model='dialog_info.time' placeholder='持续时间'>
                        <el-option v-for='data in banana_text' :key='data.key' :label='data.name'
                                   :value='data.value'/>
                    </el-select>
                </el-form-item>
            </el-form>
            <el-form>
                <el-form-item label="封禁理由">
                    <el-input type="textarea" rows="3" v-model="dialog_info.reason"></el-input>
                </el-form-item>
            </el-form>
            <el-button type="primary">确定</el-button>
        </el-dialog>
    </el-row>

</template>

<script>
import {format_time, format_permiss} from "../api/format";
import {RequestAccountList, RequestDeleteAccount} from "../api/account";
import {ElMessage, ElMessageBox} from "element-plus";
import {request_city_list} from "../api/activity";


export default {
    data() {
        return {
            query_info: {
                nick : "",
                open_id: "",
                page: 1,
                total: 0,
                size: 10,
            },
            dialog_info: {
                account: "",
                reason: "",
                time: 0,
                target_time: ""
            },
            user_info: {
                show: false,
                data: null
            },
            city_info: {
                list: [],
                city_id: 0,
            },
            account_list: [],
            banana_text: [
                {
                    key: 1,
                    value: 600,
                    name: '十分钟'
                },
                {
                    key: 2,
                    name: '一小时',
                    value: 3600
                },
                {
                    key: 3,
                    name: '一天',
                    value: 86400
                },
                {
                    key: 4,
                    name: '三天',
                    value: 86400 * 3
                },
                {
                    key: 5,
                    name: '七天',
                    value: 86400 * 7
                }
            ]

        }
    },
    methods: {
        format_time: format_time,
        format_permiss: format_permiss,
        format_sex(row, column, cellValue) {
            return cellValue === 1 ? "男" : "女"
        },

        format_vip_type(row, column, cellValue) {
            switch (cellValue) {
                case 0:
                    return "普通用户";
                case 2:
                    return "永久会员"
                case 3:
                    return "普通群"
                case 1: {
                    const now_time = new Date().getTime() / 1000
                    if (row.vip_time && row.vip_time > now_time) {
                        return "年卡会员"
                    }
                    return "过期会员"
                }
            }
            return "未知"
        },

        on_btn_look(userId) {
            localStorage.setItem("role_id", userId)
            this.$router.push("/role")
        },

        async on_btn_delete(open_id) {
            await ElMessageBox.confirm('确定删除账号吗？', '提示', {
                type: 'error'
            });
            try {
                const response = await RequestDeleteAccount(open_id)
                if (response.data.code === 0) {
                    this.on_btn_query()
                    ElMessage.success("删除账号成功")
                    return
                }
                ElMessage.error("删除账号失败")
            } catch (e) {
                ElMessage.error("请检查网络")
            }
        },

        query_activity_list(user_id) {

        },

        on_btn_bannend(account) {
            this.dialog_info.show = true
            this.dialog_info.account = account
        },

        handlePageChange(page) {
            this.query_info.page = page
            this.on_btn_query()
        },

        on_btn_query() {
            this.account_list = []
            //this.query_info.total = 0
            const page = this.query_info.page
            const open_id = this.query_info.open_id
            let city_id = this.city_info.city_id
            if (open_id && open_id.length > 0) {
                city_id = 0
            }
            const request = {
                page: page,
                open_id: open_id,
                city_id: city_id
            }
            if(this.query_info.nick.length > 0)
            {
                request.nick = this.query_info.nick
            }

            RequestAccountList(request).then(response => {
                const data = response.data.data
                if (response.data.code === 0) {
                    if (Array.isArray(data.list) && data.list.length > 0) {
                        this.account_list = data.list
                        for (let i = 0; i < this.account_list.length; i++) {
                            const info = this.account_list[i]

                            if(!info.nick || info.nick.length <= 0)
                            {
                                info.nick = "暂未设置"
                            }

                            if(!info.city_name || info.city_name.length <= 0)
                            {
                                info.city_name = "未知"
                            }

                            if(!info.desc || info.desc.length <= 0)
                            {
                                info.desc = "暂未设置"
                            }
                            //https://yy-server-log.oss-cn-beijing.aliyuncs.com/boy.png
                            if(!info.icon || info.icon.length <= 0)
                            {
                                info.icon = "https://yy-server-log.oss-cn-beijing.aliyuncs.com/boy.png"
                            }
                        }
                    }
                    this.query_info.total = data.count || 0
                    return
                }
                ElMessage.error("拉取数据失败")
            }).catch(() => {
                ElMessage.error("请检查网络")
            })
        }
    },
    async mounted() {
        this.city_info.list = []
        const response = await request_city_list()
        if (response.data.code === 0) {
            this.city_info.list.push({
                value: 0,
                label: "全部"
            })
            for (let i = 0; i < response.data.data.length; i++) {
                const city_data = response.data.data[i]
                this.city_info.list.push({
                    value: city_data.code,
                    label: `${city_data.parent_name}-${city_data.name}`
                })
            }
            this.city_info.city_id = 0
        }
        this.on_btn_query()
    }
}

</script>

<style scoped>

</style>
