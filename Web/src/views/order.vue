<template>

    <el-row style='padding-left:15px;padding-top:10px'>
        <el-form inline="inline">
            <el-form-item label="状态" style="width: 200px;">
                <el-select v-model="query.status">
                    <el-option v-for="item in order_status"
                               :key="item.name" :label="item.name" :value="item.value"></el-option>
                </el-select>
            </el-form-item>
            <el-form-item label="城市" style="width: 200px;">
                <el-select v-model="city.city" placeholder="请选择" @change="on_select_change">
                    <el-option
                        v-for="item in city.list"
                        :key="item.value"
                        :label="item.label"
                        :value="item.value">
                    </el-option>
                </el-select>
            </el-form-item>
            <el-form-item label="类型" style="width: 200px;">
                <el-select v-model="query.type">
                    <el-option v-for="item in order_type"
                               :key="item.name" :label="item.name" :value="item.value"></el-option>
                </el-select>
            </el-form-item>
            <el-form-item label="订单号">
                <el-input v-model="query.id"></el-input>
            </el-form-item>
            <el-form-item label="产品ID">
                <el-input v-model="query.product_id"></el-input>
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
            <el-table-column label="用户ID" prop="user_id" width="110rpx" show-overflow-tooltip>
                <template #default="scope">
                    <el-link type="primary" @click='navigateTo(scope.row.user_id)'>{{ format_user_name(scope.row.user_id) }}</el-link>
                </template>
            </el-table-column>
            <el-table-column label="商品ID" prop="product_id" width="80rpx"></el-table-column>
            <el-table-column label="订单描述" prop="desc" show-overflow-tooltip></el-table-column>
            <el-table-column label="城市" prop="city" width="90rpx" :formatter="format_order_city"></el-table-column>
            <el-table-column label="订单类型" prop="type" width="90rpx" :formatter="format_order_type"></el-table-column>
            <el-table-column label="订单金额" width="100rpx">
                <template #default="scope">
                    {{ "¥" + (scope.row.price / 100).toFixed(2) }}
                </template>
            </el-table-column>

            <el-table-column label="退款金额" width="100rpx">
                <template #default="scope">
                    {{ "¥" + (scope.row.refund_price / 100).toFixed(2) }}
                </template>
            </el-table-column>
            <el-table-column label="分享者ID" prop="inviter_id" width="110rpx" show-overflow-tooltip>
                <template #default="scope">
                    <el-link v-if="scope.row.inviter_id > 0" type="primary" @click='navigateTo(scope.row.inviter_id)'>{{ format_user_name(scope.row.inviter_id) }}</el-link>
                    <text v-else>{{ "无" }}</text>
                </template>
            </el-table-column>

            <el-table-column label="佣金" prop="user_id" width="80rpx">
                <template #default="scope">
                    {{ "¥" + (scope.row.commission / 100).toFixed(2) }}
                </template>
            </el-table-column>
            <el-table-column label="创建时间" prop="create_time" show-overflow-tooltip :formatter="format_time"></el-table-column>
            <el-table-column label="订单备注" prop="custom" show-overflow-tooltip></el-table-column>

            <el-table-column label="操作">
                <template #default="scope">
                    <el-button type="primary" @click="on_pay_order(scope.row._id)">退款</el-button>
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
import {RefundOrder, RequestOrderList} from "../api/server";
import {format_time} from "../api/format";
import {ElMessage, ElMessageBox} from "element-plus";
import {request_city_list} from "../api/activity";
import {httpRequest} from "../utils/httpRequest";

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
                product_id : "",
                page: 1,
                size: 10,
                count: 0,
                status: -1,
                type : 0,
            },
            order_status: [
                {name: "所有", value: -1},
                {name: "已支付", value: 2},
                {name: "待退款", value: 3},
                {name: "已退款", value: 4},
                {name: "已确认", value: 6}
            ],
            order_type: [
                {
                    name : "全部",
                    value : 0,
                },
                {
                    name : "会员订单",
                    value: 1,
                },
                {
                    name : "活动订单",
                    value: 2
                }
            ],
            user_info_list : { }
        }
    },
    methods: {
        format_time: format_time,
        navigateTo(id) {
            localStorage.setItem("role_id", id)
            this.$router.push("/role")
        },
        async on_btn_query() {
            this.from.list = []
            const city = this.city.city
            const product_id = this.query.product_id
            const response = await RequestOrderList(this.query.page, this.query.status, this.query.id, city, product_id, this.query.type);
            this.query.count = 0
            let item_list = []
            const user_list = new Set();
            if (response.data.code === 0) {
                const data = response.data.data
                if (Array.isArray(data.list)) {
                    item_list = data.list
                    const now_time = new Date().getTime() / 1000
                    for (let i = 0; i < item_list.length; i++) {
                        const item = item_list[i]
                        if (item.status === 1 && now_time >= item.over_time) {
                            item.status = 5;
                        }
                        user_list.add(item.user_id);
                        if(item.inviter_id && item.inviter_id > 0)
                        {
                            user_list.add(item.inviter_id)
                        }
                    }
                    this.query.count = data.count
                }
            }
            const list = []
            for (const userId of user_list) {
                if(!this.user_info_list[userId])
                {
                    list.push(userId);
                }
            }
            const response1 = await httpRequest.POST("/user/batch", {
                list : list
            })
            if(response1.data.code === 0 && Array.isArray(response1.data.data))
            {
                for (const userInfo of response1.data.data) {
                    this.user_info_list[userInfo.user_id] = userInfo
                }
            }
            this.from.list = item_list
        },
        handlePageChange(page) {
            this.query.page = page
            this.on_btn_query()
        },
        format_order_type(row, col, value) {
            switch (value) {
                case 1:
                    return "会员订单"
                case 2:
                    return "活动订单"
            }
            return "未知类型"
        },

        format_order_city(row, col, value) {
            if(value === 1) {
                return "斜杠青年"
            }
            const name = this.city_list[value]
            if(name) {
                return name;
            }
            return "未知"
        },

        format_user_name(userId) {
            const userInfo = this.user_info_list[userId]
            if(userInfo && userInfo.nick) {
                return userInfo.nick
            }
            return "";
        },

        format_order_status(row, col, value) {
            if(value === 4) {
                if(row.price === row.refund_price) {
                    return "全额退款"
                }
                return "部分退款"
            }
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
            await ElMessageBox.confirm('确定给订单退款吗？', '提示', {
                type: 'warning'
            });
            RefundOrder(order_id).then(response => {
                const result = response.data
                if (result.code !== 0) {
                    ElMessage.error(result.error)
                    return
                }
                ElMessage.success("退款成功")
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
