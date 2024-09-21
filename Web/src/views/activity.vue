<template>
    <h1>活动列表</h1>
    <el-form inline="inline">
        <el-form-item title="状态" style="width: 200px">
            <el-select v-model="page_info.status" placeholder="请选择" @change="on_select_change">
                <el-option
                    v-for="item in status_list"
                    :key="item.value"
                    :label="item.text"
                    :value="item.value">
                </el-option>
            </el-select>
        </el-form-item>
        <el-form-item title="城市" style="width: 200px">
            <el-select v-model="city.city" placeholder="请选择" @change="on_select_change">
                <el-option
                    v-for="item in city.list"
                    :key="item.value"
                    :label="item.label"
                    :value="item.value">
                </el-option>
            </el-select>
        </el-form-item>
        <el-form-item>
            <el-button type="primary" @click="pull_activity_list">刷新</el-button>
        </el-form-item>
    </el-form>

    <el-table :data="page_info.list" border class="table" ef="multipleTable"
              header-cell-class-name="table-header">
<!--        <el-table-column label="图标">-->
<!--            <template #default="scope">-->
<!--                <el-image :src="scope.row.url" style="border-radius: 10px"></el-image>-->
<!--            </template>-->
<!--        </el-table-column>-->
        <el-table-column label="ID" prop="_id" width="60%"></el-table-column>
        <el-table-column label="城市" prop="city" width="80%" :formatter="format_city"></el-table-column>
        <el-table-column label="标题" prop="title" width="400%" show-overflow-tooltip style="width: 20%"></el-table-column>
        <el-table-column label="状态" prop="status" width="80%" :formatter="format_status"></el-table-column>
        <el-table-column label="价格" >
            <template #default="scope"> {{ "¥" + (scope.row.price / 100).toFixed(2) }}</template>
        </el-table-column>
        <el-table-column label="人数" prop="cur_num" width="80%"></el-table-column>
        <el-table-column label="开始时间" prop="start_time" :formatter="format_time" show-overflow-tooltip></el-table-column>
        <el-table-column label="结束时间" prop="stop_time" :formatter="format_time" show-overflow-tooltip></el-table-column>
        <el-table-column label="操作">
            <template #default="scope">
                <el-button type="primary" plain @click="on_btn_look(scope.row)">查看</el-button>
            </template>
        </el-table-column>
    </el-table>
    <div class="pagination">
        <el-pagination
            background
            layout="total, prev, pager, next"
            :current-page="page_info.page"
            :page-size="page_info.size"
            :total="page_info.total"
            @current-change="handlePageChange"
        ></el-pagination>
    </div>
    <el-dialog v-model="look_info.show" :title="look_info.data.title">
        <activity-preview :info="look_info.data"></activity-preview>
    </el-dialog>
</template>

<script>

import {format_time} from '../api/format'
import {request_activity_list, request_city_list} from "../api/activity"
import {ElMessage} from "element-plus";
import ActivityPreview from "./ActivityPreview.vue";

export default {
    components: {
        ActivityPreview
    },
    data() {
        return {
            look_info: {
                data: {
                    title: "",
                },
                show: false
            },
            page_info: {
                page: 1,
                total: 0,
                size: 10,
                list: [],
                status: 0,
            },
            city_list: {},
            city : {
              list : [],
              city : 0
            },
            status_list: [
                {
                    value: -1,
                    text: "所有"
                },
                {
                    value: 0,
                    text: "报名中",
                },
                {
                    value: 1,
                    text: "进行中"
                },
                {
                    value: 2,
                    text: "已结束"
                }
            ]
        }
    },
    methods: {
        on_btn_look(row) {
            this.look_info.data = row
            this.look_info.show = true
        },

        on_select_change(value) {
            this.page_info.page = 1
            this.pull_activity_list()
        },
        handlePageChange(page) {

            this.page_info.page = page
            this.pull_activity_list()
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
                case 4:
                    return "待审核"
                case 5:
                    return "待处理"
            }
            return "未知"
        },
        format_time: format_time,
        format_city(row, column, cellValue) {
            if(cellValue === 0) {
                return "全国"
            }
            return  this.city_list[cellValue]
        },
        pull_activity_list() {
            this.page_info.list = []
            request_activity_list(this.page_info.page, this.page_info.status, this.city.city).then(result => {
                const response = result.data
                if (response.code !== 0) {
                    ElMessage.error("拉取活动列表失败")
                    return
                }
                if (Array.isArray(response.data.list)) {
                    this.page_info.total = response.data.count
                    this.page_info.list = response.data.list
                    for (const item of this.page_info.list) {
                        item.cur_num = 0
                        if (Array.isArray(item.users)) {
                            item.cur_num = item.users.length
                        }
                    }
                }

            }).catch(() => {
                ElMessage.error("请检查网络")
            })
        }
    },
    mounted() {
        request_city_list().then(response => {
            const result = response.data
            this.city.list = [{
                label : "全国",
                value : 0
            }]
            this.city_list[0] = "全部"
            if (result.code === 0) {
                const cityList = result.data
                for (let item in cityList) {
                    const cityItem = {};
                    if (cityList[item].name.indexOf("市辖") > -1) {
                        cityItem.label = cityList[item].parent_name
                    } else {
                        cityItem.label = cityList[item].name
                    }
                    cityItem.value = cityList[item].code;
                    this.city.list.push(cityItem);
                    this.city_list[cityItem.value] = cityItem.label
                }
            }
        })

        this.pull_activity_list()
    }
}

</script>

<style scoped>

</style>
