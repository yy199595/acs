<template>

    <el-form :rules="rules" :model="add_activity_info.info">
        <el-form inline="inline">
            <el-form-item label="地区">
                <el-select v-model="add_activity_info.info.city" @change="on_select_city"
                           placeholder="选择市" :disabled="from.disable_city">
                    <el-option v-for="item in add_activity_info.open_city_list"
                               :key="item.code" :label="item.parent_name + item.name" :value="item.code"></el-option>
                </el-select>
            </el-form-item>

            <el-form-item label="地址">
                <el-cascader :props="{ label: 'name', value: 'name' }"
                             v-model="this.add_activity_info.values"
                             :options="add_activity_info.city_list" @change="on_select_addr">
                </el-cascader>
            </el-form-item>

            <el-form-item>
                <el-upload accept=".png,.jpg" :http-request="on_upload_icon" :show-file-list="false">
                    <el-button type="primary" plain>上传封面</el-button>
                </el-upload>
            </el-form-item>
        </el-form>

        <el-form inline="inline">
            <el-form-item label="图标" v-if="add_activity_info.info.url.length > 0">
                <el-image :src="add_activity_info.info.url" style="width: 300px;height: 200px"></el-image>
            </el-form-item>
        </el-form>

        <el-form-item label="地址" prop="address">
            <el-input style="width: 650px" type="text" placeholder="请输入门牌号"
                      v-model="add_activity_info.info.address">
                <template v-slot:prepend v-if="add_activity_info.city_prepend.length > 0">
                    <span class="prefix-text">{{ add_activity_info.city_prepend }}</span>
                </template>
            </el-input>
        </el-form-item>

        <el-form-item label="标题" prop="title">
            <el-input style="width: 650px" v-model="add_activity_info.info.title"
                      placeholder="输入活动标题，会展示在首页"></el-input>
        </el-form-item>

        <el-form inline="inline" :rules="rules2" :model="add_activity_info.info">
            <el-form-item label="标签" prop="tag">
                <el-input v-model="add_activity_info.info.tag" placeholder="输入活动标签"></el-input>
            </el-form-item>
            <el-form-item label="普通价" prop="price">
                <el-input v-model="add_activity_info.info.price" placeholder="输入普通价格"></el-input>
            </el-form-item>
            <el-form-item label="会员价" prop="vip_price">
                <el-input v-model="add_activity_info.info.vip_price" placeholder="输入会员价格"></el-input>
            </el-form-item>
            <el-form-item label="限制人数" prop="max_num">
                <el-input v-model="add_activity_info.info.max_num" placeholder="输入活动人数,0不限制"></el-input>
            </el-form-item>
        </el-form>

        <el-form-item label="活动描述" prop="content">
            <text-editor :on_text="on_input_text"></text-editor>
        </el-form-item>

        <el-form inline="inline">
            <el-form-item label="开始时间">
                <el-date-picker type="datetime" placeholder="选择活动开始时间"
                                v-model="add_activity_info.info.start_time">

                </el-date-picker>
            </el-form-item>
            <el-form-item label="结束时间">
                <el-date-picker type="datetime" placeholder="选择活动结束时间"
                                v-model="add_activity_info.info.stop_time"></el-date-picker>
            </el-form-item>

            <el-form-item label="报名截止时间">
                <el-date-picker type="datetime" placeholder="选择活动结束时间"
                                v-model="add_activity_info.info.sign_time"></el-date-picker>
            </el-form-item>
        </el-form>
        <el-button type="primary" @click="on_btn_publish">{{ from.btn_name }}</el-button>
        <el-button type="primary" @click="on_btn_preview">预览</el-button>
    </el-form>

    <el-dialog v-model="preview_info.show" title='活动预览'>
        <activity-preview :info="preview_info.data"></activity-preview>
    </el-dialog>
</template>

<script>

import {pub_activity, request_city_list, request_city_info, change_activity} from "../api/activity";
import {ElMessage} from "element-plus";
import {oss_upload} from "../api/upload";
import TextEditor from "./TextEditir.vue"
import ActivityPreview from "./ActivityPreview.vue";
import {get_city_list} from "../api/city";

export default {
    components: {
        TextEditor,
        ActivityPreview
    },
    data() {
        return {
            from: {
                type: 1,
                btn_name: "发布",
                disable_city: false
            },
            add_activity_info: {
                show: false,
                info: {
                    _id: 0,
                    city: 5001,
                    title: "",
                    code: "",
                    address: "",
                    content: "",
                    price: "",
                    vip_price: "",
                    url: "",
                    tag: "",
                    urls: [],
                    sign_time: new Date(),
                    start_time: new Date(),
                    stop_time: new Date(),
                    cur_num: 0,
                    max_num: "",
                },
                tag: 1,
                values: [],
                name: "name",
                city_name: "",
                city_prepend: "",
                tag_list: [],
                city_list: [],
                open_city_list: [], //开发城市列表
            },
            image_list: [],
            preview_info: {
                show: false,
                data: {}
            },
            rules: {
                title: [
                    {required: true, message: "输入活动标题", trigger: "blur"},
                    {min: 5, max: 50, message: "标题长度在5到50个字左右", trigger: "blur"}
                ],
                content: [
                    {required: true, message: "输入活动描述", trigger: "blur"},
                    {min: 10, max: 500, message: "标题长度在5到50个字左右", trigger: "blur"}
                ],
                address: [
                    {required: true, message: "输入活动地址", trigger: "blur"},
                ]
            },
            rules2: {
                tag: [
                    {required: true, message: "输入活动标签", trigger: "blur"},
                ],
                price: [
                    {required: true, message: "输入普通价格", trigger: "blur"},
                ],
                vip_price: [
                    {required: true, message: "输入会员价格", trigger: "blur"},
                ],
                max_num: [
                    {required: true, message: "输入活动人数", trigger: "blur"},
                ],
            }

        }
    },
    methods: {
        on_input_text(text) {
            this.add_activity_info.info.content = text
        },
        on_btn_preview() {
            this.preview_info.show = true
            this.preview_info.data = this.pack_activity_info()
        },

        async on_select_city() {

        },

        on_select_addr(value) {
            this.add_activity_info.city_prepend = ""
            for (let i = 0; i < this.add_activity_info.values.length; i++) {
                const val = this.add_activity_info.values[i]
                this.add_activity_info.city_prepend += val
            }
        },
        async on_upload_icon(options) {
            const file = options.file
            if (!file) {
                return false
            }
            const url = await oss_upload(file)
            if (url != null) {
                this.add_activity_info.info.url = url
                ElMessage.success("上传图标成功")
                return true
            }
            ElMessage.error("上传图标失败")
            return false
        },
        pack_activity_info() {
            let request = {
                urls: [],
                _id: this.add_activity_info.info._id,
                url: this.add_activity_info.info.url,
                tag: this.add_activity_info.info.tag,
                title: this.add_activity_info.info.title,
                price: parseInt(this.add_activity_info.info.price),
                content: this.add_activity_info.info.content,
                max_num: parseInt(this.add_activity_info.info.max_num),
                vip_price: parseInt(this.add_activity_info.info.vip_price),
                city: this.add_activity_info.info.city,
                sign_time: this.add_activity_info.info.sign_time.getTime() / 1000,
                stop_time: this.add_activity_info.info.stop_time.getTime() / 1000,
                start_time: this.add_activity_info.info.start_time.getTime() / 1000,
                address: this.add_activity_info.city_prepend + this.add_activity_info.info.address,
            }
            const nowTime = new Date().getTime() / 1000
            if (request.start_time === 0) {
                request.start_time = nowTime;
            }
            return request
        },
        on_btn_publish() {

            const request = this.pack_activity_info()
            if (request.url.length === 0) {
                ElMessage.error("请上传活动图标")
                return;
            }
            if (request.tag.length === 0) {
                ElMessage.error("请输入活动标签")
                return;
            }
            if (request.address.length === 0) {
                ElMessage.error("请选择活动地址")
                return;
            }
            if (request.title.length === 0) {
                ElMessage.error("请输入活动标题")
                return;
            }
            if (request.content.length === 0) {
                ElMessage.error("请输入活动描述")
                return;
            }

            if (request.start_time >= request.stop_time) {
                window.alert("开始时间不能大于结束时间")
                return
            }
            const now_time = new Date().getTime() / 1000
            if (now_time >= request.start_time) {
                window.alert("开始时间不能小于当前时间")
                return
            }
            switch (this.from.type) {
                case 1: //发布活动
                {
                    pub_activity(request).then(result => {
                        if (result.data.code !== 0) {
                            ElMessage.error("发布活动失败:" + result.data.error)
                            return
                        }
                        ElMessage.success("发布活动成功")
                    })
                    this.add_activity_info.show = false
                    break
                }
                case 2:  //更新活动
                {
                    change_activity(request).then(result => {
                        if (result.data.code !== 0) {
                            ElMessage.error("更新活动失败:" + result.data.error)
                            return
                        }
                        ElMessage.success("更新活动成功")
                    })
                    break
                }
            }
        },

        reset_activity_info() {
            const activity_str = localStorage.getItem("activity_json") //更新活动
            if (activity_str) {
                this.add_activity_info.info = JSON.parse(activity_str)
                const activity_type = localStorage.getItem("activity_type")
                if(activity_type) {
                    this.from = JSON.parse(activity_type);
                    localStorage.removeItem("activity_type")
                }
                const t3 = this.add_activity_info.info.sign_time
                const t2 = this.add_activity_info.info.stop_time
                const t1 = this.add_activity_info.info.start_time

                this.add_activity_info.info.sign_time = new Date(t3 * 1000)
                this.add_activity_info.info.stop_time = new Date(t2 * 1000)
                this.add_activity_info.info.start_time = new Date(t1 * 1000)

                localStorage.removeItem("activity_json")
                return
            }
            const date = new Date()
            date.setHours(0)
            date.setMinutes(0)
            date.setSeconds(0)
            this.add_activity_info.info.price = ""
            this.add_activity_info.info.max_num = ""
            this.add_activity_info.info.content = ""
            this.add_activity_info.info.address = ""
            this.add_activity_info.info.start_time = date
            this.add_activity_info.info.stop_time = date
            this.add_activity_info.tag = 1
        },
        init_city_name() {
            const city = this.add_activity_info.info.city
            const index = this.add_activity_info.open_city_list.findIndex(iter => iter.code === city)
            if (index !== -1) {
                this.add_activity_info.city_name = this.add_activity_info.open_city_list[index].name
            }
        }
    },
    async mounted() {

        this.reset_activity_info()
        const response = await request_city_list()
        const response2 = await request_city_info(5001)

        this.add_activity_info.city_list = response2.data.data

        this.add_activity_info.city_list = await get_city_list()
        this.add_activity_info.open_city_list = response.data.data
        for (let i = 0; i < this.add_activity_info.open_city_list.length; i++) {
            const item = this.add_activity_info.open_city_list[i]
            console.log(item.name)
            if (item.name === "市辖区") {
                this.add_activity_info.open_city_list[i].name = ""
            }
        }
        this.add_activity_info.open_city_list.push({
            name : "全国",
            parent_name : "",
            code : 0
        })
        this.add_activity_info.open_city_list.sort((item1, item2) => {
            return item1.name.length - item2.name.length
        })

        this.add_activity_info.show = true
        this.init_city_name();
    }
}

</script>

<style scoped>

.rich-text-container img {
    max-width: 100%; /* 限制图片的最大宽度为父容器的宽度 */
    height: auto; /* 保持图片的纵横比 */
}
</style>
