<template>
    <div>
        <el-row :gutter="20">
            <el-col :span="5">
                <el-card shadow="hover" class="mgb20" style="height: 41vh">
                    <div class="user-info-cont">
                        <Avatar type="home" :userName="user.name"/>
                    </div>
                    <div style="font-size: 20px; color: #dd4a68">
                        <span class="info-label">权限：</span>
                        <span>{{ format_permiss(null, null, user.permission) }}</span>
                    </div>
                    <div style="font-size: 20px; color: #dd4a68">
                        <span class="info-label">城市：</span>
                        <span>{{ user.city_name }}</span>
                    </div>
                    <div class="user-info-list" style="font-size: 20px; color: #20a0ff">
                        <span class="info-label">登录IP：</span>
                        <span style="color: #dd4a68">{{ user.login_ip }}</span>
                    </div>
                    <div class="user-info-list" style="font-size: 20px; color: #79bbff">
                        <span class="info-label">注册时间：</span>
                        <span style="color: #67c23a;">{{ timeToStrTime(user.create_time * 1000) }}</span>
                    </div>
                    <div class="user-info-list" style="font-size: 20px; color: #a0cfff">
                        <span class="info-label">登录时间：</span>
                        <span style="color: #67c23a;">{{ timeToStrTime(user.login_time * 1000) }}</span>
                    </div>
                </el-card>
                <el-button @click="show_view=true">小姐姐视频</el-button>
<!--                <el-card shadow="hover" class="mgb20" style="height: 42vh">-->
<!--                    <video width="250" height="350" controls autoplay>-->
<!--                        <source src="http://api.yujn.cn/api/zzxjj.php?type=video">-->
<!--                    </video>-->
<!--                </el-card>-->
                <el-dialog v-model="show_view" title="小姐姐视频">
                    <el-button @click="index++">刷新</el-button>
                    <video :key="index" width="500" height="600" controls autoplay src="http://api.yujn.cn/api/zzxjj.php?type=video">

                    </video>
                </el-dialog>
            </el-col>
            <el-col :span="19">
                <el-card shadow="hover" style="height: 85vh">
                    <h3>操作记录</h3>
                    <el-table :data="tab.list" border>
                        <el-table-column label="用户" prop="name"></el-table-column>
                        <el-table-column label="方法" prop="method"></el-table-column>
                        <el-table-column label="路径" prop="url"></el-table-column>
                        <el-table-column label="描述" prop="desc"></el-table-column>
                        <el-table-column label="时间" prop="time" :formatter="format_time"></el-table-column>
                        <el-table-column label="请求" prop="request" show-overflow-tooltip></el-table-column>
                        <el-table-column label="返回" prop="response" show-overflow-tooltip></el-table-column>
                    </el-table>

                    <div class="pagination">
                        <el-pagination
                            background
                            layout="total, prev, pager, next"
                            :current-page="tab.page.index"
                            :page-size="tab.page.count"
                            :total="tab.page.total"
                            @current-change="handlePageChange"
                        ></el-pagination>
                    </div>
                </el-card>
            </el-col>
        </el-row>

    </div>
</template>

<script>

import {format_permiss, format_time} from "../api/format";
import {RequestOptRecord} from "../api/login";
import Avatar from '../views/Avatar.vue';


export default {
    name: "welcome",
    components: {
        Avatar
    },
    data() {
        return {
            tab: {
                list: [],
                page: {
                    index: 1,
                    count: 8,
                    total: 0
                },
            },
            index : 1,
            show_view : false,
            user: {
                name: "",
                login_ip: "",
                city_name: "",
                permission: 0,
                login_time: 0,
                create_time: 0,
            },
            format_time: format_time,
            format_permiss: format_permiss
        }
    },
    methods: {
        timeToStrTime(timestamp) {
            const date = new Date(timestamp);
            return  date.toLocaleString('zh-CN', {timeZone: 'Asia/Shanghai', hour12: false});
        },
        handlePageChange(page) {
            this.tab.page.index = page
            this.query_operator_record().then(()=> {

            })
        },
        async query_operator_record() {
            const response = await RequestOptRecord(this.tab.page.index)
            const result = response.data
            if (result.code === 0) {
                this.tab.list = result.data.list
                this.tab.page.total = result.data.count
            }
        }
    },
    async mounted() {
        const user_json = localStorage.getItem("user_info")
        if (user_json && user_json.length > 0) {
            this.user = JSON.parse(user_json)
        }
        console.log(this.user)
        // const response1 = await axios.get("https://findmyip.net/api/ipinfo.php?ip=" + this.user.login_ip)
        // if(response1.status === 200 && response1.data.data)
        // {
        //     const result = response1.data.data["API_1"]
        //     this.user.login_city = `${result.country}-${result.city}`
        // }
        this.query_operator_record().then(()=> {

        })
    }
}
</script>

<style>
.preserve-space {
    white-space: pre;
}
</style>

