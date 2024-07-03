<template>
    <h3>会员卡列表</h3>
    <el-row style='padding-left:15px;padding-top:30px'>
        <el-form inline="inline">
            <el-form-item>
                <el-button type="primary" @click="open_dialog">添加</el-button>
            </el-form-item>
        </el-form>
        <el-table :data="from.cards" border>
            <el-table-column label="图标">
                <template #default="scope">
                    <el-image :src="scope.row.icon"></el-image>
                </template>
            </el-table-column>
            <el-table-column label="ID" prop="_id"></el-table-column>
            <el-table-column label="名字" prop="name"></el-table-column>
            <el-table-column label="价格" prop="price"></el-table-column>
            <el-table-column label="有效期" prop="time" :formatter="format_time"></el-table-column>
            <el-table-column label="描述" prop="desc"></el-table-column>
            <el-table-column label="操作">
                <template #default="scope">
                    <el-button type="primary" @click="edit_card(scope.row)">修改</el-button>
                    <el-button type="danger" @click="del_card(scope.row._id)">删除</el-button>
                </template>
            </el-table-column>
        </el-table>

        <el-dialog title="添加会员卡" v-model="from.input.show" width="50%">
            <el-form>
                <el-form-item label="图标">
                    <el-upload accept=".png, .jpg" v-if="from.input.data.icon.length === 0"
                               :http-request="on_http_request" :show-file-list="false">
                        <el-button type="primary">点击上传</el-button>
                    </el-upload>
                    <el-image v-if="from.input.data.icon.length > 0" :src="from.input.data.icon"></el-image>
                </el-form-item>
                <el-form-item label="名字" label-width="60px">
                    <el-input v-model="from.input.data.name" placeholder="输入会员卡名字"></el-input>
                </el-form-item>
                <el-form-item label="价格" label-width="60px">
                    <el-input-number v-model="from.input.data.price" min="1"></el-input-number>
                </el-form-item>
                <el-form-item label="有效期" label-width="60px">
                    <el-select v-model="from.input.data.time">
                        <el-option v-for="item in time_list"
                                   :key="item.name" :label="item.name" :value="item.time"></el-option>
                    </el-select>
                </el-form-item>
                <el-form-item label="描述" label-width="60px">
                    <el-input type="textarea" rows="5" v-model="from.input.data.desc"
                              placeholder="输入会员卡描述权益等"></el-input>
                </el-form-item>
            </el-form>
            <el-button type="primary" @click="add_card">添加</el-button>
        </el-dialog>
    </el-row>
</template>

<script>
import {CreateVipCard, DeleteVipCard, RequestCards} from "../api/server";
import {ElMessage, ElMessageBox} from "element-plus";
import {oss_upload} from "../api/upload";

export default {
    data() {
        return {
            from: {
                cards: [],
                input: {
                    data: {
                        name: "",
                        price: 1,
                        score : 1,
                        month_score : 1,
                        time: 0,
                        desc: "",
                        icon: "",
                    },
                    show: false
                },
            },
            time_list: [
                {
                    time: 7 * 86400,
                    name: "七天"
                },
                {
                    time: 30 * 86400,
                    name: "一个月"
                },
                {
                    time: 30 * 86400 * 3,
                    name: "三个月"
                },
                {
                    time: 30 * 86400 * 3,
                    name: "一年"
                },
                {
                    time: -1,
                    name: "永久"
                }
            ]
        }
    },
    mounted() {
        this.query_cards()
    },
    methods: {
        open_dialog() {
            this.from.input.show = true
            this.from.input.data.desc = ""
            this.from.input.data.name = ""
            this.from.input.data.price = 10
            this.from.input.data.icon = ""
            this.from.input.data.time = this.time_list[0].time
        },
        query_cards() {
            this.from.cards = []
            RequestCards().then(response => {
                if (Array.isArray(response.data.data)) {
                    this.from.cards = response.data.data
                    console.log(this.from.cards)
                }
            })
        },
        async on_http_request(options) {
            const file = options.file
            if (!file) {
                return false
            }
            const url = await oss_upload(file)
            if (url != null) {
                this.from.input.data.icon = url
                ElMessage.success("上传图标成功")
                return true
            }
            ElMessage.error("上传图标失败")
            return false
        },
        add_card() {
            this.from.input.show = false
            CreateVipCard(this.from.input.data).then(response => {
                if (response.data.code === 0) {
                    this.query_cards()
                    ElMessage.success("添加会员卡成功")
                    return
                }
                ElMessage.error("添加会员卡失败")
            })
        },

        async edit_card(card_info) {

        },

        async del_card(id) {
            await ElMessageBox.confirm('确定删除会员卡吗？', '提示', {
                type: 'error'
            });
            DeleteVipCard(id).then(response => {
                const result = response.data
                if (result.code !== 0) {
                    ElMessage.success("删除会员卡成功")
                    return
                }
                ElMessage.error("删除会员卡失败")
            })
        },
        format_time(row, column, cellValue) {
            const index = this.time_list.findIndex(item => item.time === cellValue)
            return this.time_list[index].name
        }
    }
}
</script>

<style scoped>

</style>
