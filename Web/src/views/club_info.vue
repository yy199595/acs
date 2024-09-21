<template>
    <el-row style='padding-left:15px;padding-top:10px'>

        <el-form inline="inline">
            <el-form-item label="城市" style="width: 200px">
                <el-select v-model="city_info.city_id" clearable placeholder="请选择城市">
                    <el-option v-for="(item, index) in city_info.list" :key="index"
                               :label="item.label" :value="item.value"></el-option>
                </el-select>
            </el-form-item>
            <el-form-item>
                <el-button type='primary' @click='on_btn_query'>查询</el-button>
            </el-form-item>
        </el-form>

        <el-table :data="query_info.club_list" border class="table" ref="multipleTable" header-cell-class-name="table-header">
            <el-table-column label="头像" #default="scope" width="100rpx">
                <el-avatar size="large" shape="square" :src="scope.row.icon"></el-avatar>
            </el-table-column>
            <el-table-column prop="_id" label="部落ID" width="100rpx"></el-table-column>
            <el-table-column prop="nick" label="昵称" width="100rpx"></el-table-column>
            <el-table-column label="主理人" width="100rpx">
                <template #default="scope">
                    <el-link type="primary" @click='navigateTo(scope.row.user_id)'>{{ scope.row.user_id }}</el-link>
                </template>
            </el-table-column>
            <el-table-column prop="create_time" label="创建时间" :formatter="format_time" width="200rpx"></el-table-column>
            <el-table-column prop="desc" label="简介" show-overflow-tooltip></el-table-column>
            <el-table-column label="操作" width="200rpx">
                <template #default="scope">
                    <el-button type="danger" @click="dissolve_club(scope.row)" plain>解散</el-button>
                    <el-button type="success" plain>修改</el-button>
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
    </el-row>

</template>

<script>
import {format_time, format_permiss} from "../api/format";
import {request_user_info, RequestAccountList, RequestDeleteAccount} from "../api/account";
import {ElMessage, ElMessageBox} from "element-plus";
import {dissolve_club, request_city_list, request_club_list} from "../api/activity";


export default {
    data() {
        return {
            city_info: {
                list: [],
                city_id: 0,
            },
            query_info: {
                page: 1,
                total: 0,
                size: 10,
                club_list : []
            },
        }
    },
    methods: {
        format_time: format_time,

        navigateTo(id) {
            localStorage.setItem("role_id", id)
            this.$router.push("/role")
        },

        async dissolve_club(row) {
            console.log(JSON.stringify(row))
            await ElMessageBox.confirm(`确定解散[${row.nick}]吗`, '提示', {
                type: 'error'
            });
           const response = await dissolve_club(row._id)
            if(response.data.code === 0 ){
                ElMessage.success("解散部落成功")
                return
            }
            ElMessage.error(response.data.error)
        },

        on_btn_query() {
            this.query_info.club_list = []
            const page = this.query_info.page
            request_club_list(page).then(response=> {
                if(response.data.code === 0) {
                    if(Array.isArray(response.data.data)) {
                        this.query_info.club_list = response.data.data
                        for (let i = 0; i < this.query_info.club_list.length; i++) {
                            const info = this.query_info.club_list[i]
                        }
                        this.query_info.total = response.data.count
                    }
                }
            })
        },

        handlePageChange(page) {
            this.query_info.page = page
            this.on_btn_query()
        },

    },
    async mounted() {

        this.city_info.list = []
        const response = await request_city_list()
        if(response.data.code === 0)
        {
            this.city_info.list.push({
                value : 0,
                label : "全部"
            })
            for (let i = 0; i < response.data.data.length; i++) {
                const city_data = response.data.data[i]
                this.city_info.list.push({
                    value : city_data.code,
                    label : `${city_data.parent_name}-${city_data.name}`
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
