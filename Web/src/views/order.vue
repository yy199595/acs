<template>

    <h3>订单列表</h3>
    <el-row style='padding-left:15px;padding-top:30px'>
        <el-form inline="inline">
            <el-form-item label="状态">
                <el-select v-model="query.status">
                    <el-option v-for="item in order_status"
                               :key="item.name" :label="item.name" :value="item.value"></el-option>
                </el-select>
            </el-form-item>
            <el-form-item title="城市">
                <el-select v-model="city.city" placeholder="请选择" @change="on_select_change">
                    <el-option
                        v-for="item in city.list"
                        :key="item.value"
                        :label="item.label"
                        :value="item.value">
                    </el-option>
                </el-select>
            </el-form-item>
            <el-form-item label="订单号">
                <el-input v-model="query.id"></el-input>
            </el-form-item>
            <el-form-item>
                <el-button type="primary" @click="on_btn_query">查询</el-button>
            </el-form-item>
        </el-form>
        <el-table :data="from.list" border>
<!--            <el-table-column label="图标">-->
<!--                <template #default="scope">-->
<!--                    <el-image :src="scope.row.icon"></el-image>-->
<!--                </template>-->
<!--            </el-table-column>-->
            <el-table-column label="订单号" prop="_id" show-overflow-tooltip></el-table-column>
            <el-table-column label="状态" prop="status" width="90rpx" :formatter="format_order_status"></el-table-column>
            <el-table-column label="用户ID" prop="user_id" width="80rpx">
                <template #default="scope">
                    <el-link type="primary" @click='navigateTo(scope.row.user_id)'>{{ scope.row.user_id }}</el-link>
                </template>
            </el-table-column>
            <el-table-column label="商品ID" prop="product_id" width="80rpx"></el-table-column>
            <el-table-column label="订单描述" prop="desc" show-overflow-tooltip></el-table-column>
            <el-table-column label="城市" prop="city" width="80rpx" :formatter="format_order_city"></el-table-column>
            <el-table-column label="订单类型" prop="type" width="90rpx" :formatter="format_order_type"></el-table-column>
            <el-table-column label="订单金额" width="100rpx">
                <template #default="scope">
                    {{ "¥" + (scope.row.price / 100).toFixed(2) }}
                </template>
            </el-table-column>
            <el-table-column label="分享者ID" prop="inviter_id" width="90rpx">
                <template #default="scope">
                    <el-link v-if="scope.row.inviter_id > 0" type="primary" @click='navigateTo(scope.row.inviter_id)'>{{ scope.row.inviter_id }}</el-link>
                    <text v-else>{{ "无" }}</text>
                </template>
            </el-table-column>

            <el-table-column label="佣金" prop="user_id" width="80rpx">
                <template #default="scope">
                    {{ "¥" + (scope.row.commission / 100).toFixed(2) }}
                </template>
            </el-table-column>
            <el-table-column label="创建时间" prop="create_time" show-overflow-tooltip :formatter="format_time"></el-table-column>
            <el-table-column label="操作">
                <template #default="scope">
                    <el-button :disabled="scope.row.status !== 2"
                               type="primary" @click="on_pay_order(scope.row._id)">退款</el-button>
                </template>
            </el-table-column>
        </el-table>

        <div class="pagination">
            <el-pagination
                background
                layout="total, prev, pager, next"
                :current-page="query.page"
                :page-size="query.size"
                :total="query.count"
                @current-change="handlePageChange"
            ></el-pagination>
        </div>
    </el-row>
</template>

<script>
import {PayOrder, RequestOrderList} from "../api/server";
import {format_time} from "../api/format";
import {ElMessage, ElMessageBox} from "element-plus";
import {request_city_list} from "../api/activity";

export default {
    data() {
        return {
            from: {
                list: [],
                city_list: []
            },
            city : {
                list : [],
                city : 0
            },
            city_list: {},
            query: {
                id: "",
                page: 1,
                size: 5,
                count: 0,
                status: -1
            },
            order_status: [
                {name: "所有", value: -1},
                {name: "已支付", value: 2},
                {name: "待退款", value: 3},
                {name: "已退款", value: 4},
                {name: "已确认", value: 6}
            ],
            order_type: []
        }
    },
    methods: {
        format_time: format_time,
        navigateTo(id) {
            localStorage.setItem("role_id", id)
            this.$router.push("/role")
        },
        on_btn_query() {
            this.from.list = []
            const city = this.city.city
            RequestOrderList(this.query.page, this.query.status, this.query.id, city).then(response => {
                this.query.count = 0
                const data = response.data.data
                if (response.data.code === 0) {
                    if (Array.isArray(data.list)) {
                        this.from.list = data.list
                        const now_time = new Date().getTime() / 1000
                        for (let i = 0; i < this.from.list.length; i++) {
                            const item = this.from.list[i]
                            if (item.status === 1 && now_time >= item.over_time) {
                                item.status = 5;
                            }
                        }
                        this.query.count = data.count
                    }
                }
            })
        },
        handlePageChange(page) {
            this.query.page = page
            this.on_btn_query()
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

        format_order_city(row, col, value) {
            const name = this.city_list[value]
            if(name) {
                return name;
            }
            return "未知"
        },

        format_order_status(row, col, value) {
            if(row.sallet_time > 0)
            {
                return "已经结算"
            }
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
        async on_pay_order(order_id) {
            await ElMessageBox.confirm('确定手动支付这个订单吗？', '提示', {
                type: 'warning'
            });
            PayOrder(order_id).then(response => {
                const result = response.data
                if (result.code !== 0) {
                    ElMessage.error(result.error)
                    return
                }
                ElMessage.success("支付订单成功")
            })
        }
    },
    async mounted() {
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
        this.on_btn_query()
    }
}
</script>

<style scoped>

</style>
