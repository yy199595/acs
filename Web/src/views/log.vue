<template>
    <h3>日志列表</h3>
    <div>
        <el-form inline="inline">
            <el-form-item label="日期">
                <el-select style='width: 200px' v-model="query.date" placeholder="选择日志" @change="on_refresh_data">
                    <el-option v-for="item in date_list" :key="item" :label="item"
                               :value="item"/>
                </el-select>
            </el-form-item>
            <el-table :data="show_log_list" border>
                <el-table-column label="日志">
                    <template #default="scope">
                        <el-link underline type="primary" :href="scope.row.url">{{ scope.row.path }}</el-link>
                    </template>
                </el-table-column>
                <el-table-column label="文件大小" prop="size" :formatter="format_size"></el-table-column>
                <el-table-column label="最后写入时间" prop="time" :formatter="format_time"></el-table-column>
                <el-table-column label="操作">
                    <template #default="scope">
                        <el-button type="primary" @click="on_btn_download(scope.row)">下载</el-button>
                    </template>
                </el-table-column>
            </el-table>

            <el-form-item>
                <el-button type="primary" v-if="query.file !==''" @change="on_change_log">刷新</el-button>
                <el-button type="success" v-if="query.file !==''">下载</el-button>
            </el-form-item>
        </el-form>
    </div>

</template>

<script setup lang="ts">
import {ElMessage} from "element-plus";
import {onMounted, ref} from "vue";
import {format_time, format_size} from "../api/format"
import {RequestDownloadLog, RequestLogList} from "../api/server";

interface LogInfo {
    url: string,
    dir: string,
    size: number,
    time: number,
    path: string
}

const date_list = ref<string[]>([])
const show_log_list = ref<LogInfo[]>([])
let log_data_list = ref<LogInfo[]>([])

const query = ref({
    date: '',
    file: ''
})


const show_log_info = ref({
    text: "",
    show: false,
    host : ""
})

const on_btn_download = async (data : any) => {
    RequestDownloadLog(data.path).then(response=> {
        const blob = new Blob([response.data], { type: 'text/plain' });
        // 创建一个临时链接
        const url = window.URL.createObjectURL(blob);
        // 创建一个 a 标签并设置 download 属性和 href 属性
        const link = document.createElement('a');
        link.href = url;
        link.setAttribute('download', data.path.replace(/\//g, '-'));
        // 点击该链接进行下载
        document.body.appendChild(link);
        link.click();
        // 完成后移除链接
        document.body.removeChild(link);
    })

}

const on_change_log = () => {

}

const on_refresh_data = () => {
    show_log_list.value = []
    for (let i = 0; i < log_data_list.value.length; i++) {
        const item = log_data_list.value[i]
        if (item.dir == query.value.date) {
            show_log_list.value.push(item)
        }
    }
}

onMounted(() => {
    RequestLogList().then(result => {
        const response = result.data
        ElMessage.success("拉取日志列表成功")
        if (response.code === 0) {
            show_log_info.value.host = response.data.host
            log_data_list.value = response.data.list as LogInfo[]
            log_data_list.value.map(item => {
                const item_infos = item.path.split("/")
                if (item_infos.length == 2) {
                    const dir = item_infos[0]
                    if (date_list.value.findIndex(name => name == dir) == -1) {
                        date_list.value.push(dir)
                    }
                    item.dir = dir
                    item.url = item.path
                    query.value.date = dir
                }
            })
            console.log(log_data_list)
            on_refresh_data();
            return
        }
    }).catch((err) => {
        ElMessage.error("拉取日志列表失败")
        console.error(err)
    })
})

</script>

<style scoped>

.input_style {
    font-size: 20px;
    white-space: pre;
    height: border-box;
    position: marker;
}

</style>
