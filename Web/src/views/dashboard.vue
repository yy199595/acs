<template>
    <div class="container">
<!--        <span style="color: #cf9236">服务器列表</span>-->
<!--        <el-form inline="inline">-->
<!--            <el-form-item label="类型" style="width: 100rpx">-->
<!--                <el-select placeholder="选择类型" v-model="server_name" style="width: 150px;">-->
<!--                    <el-option v-for="name in server_name_list" :key="name" :label="name"></el-option>-->
<!--                </el-select>-->
<!--            </el-form-item>-->
<!--            <el-form-item>-->
<!--                <el-button type="primary" @click="GetServerList">刷新</el-button>-->
<!--            </el-form-item>-->
<!--        </el-form>-->
        <el-table class="table" border :data="server_list" style="font-size: 20px">
            <el-table-column type="expand">
                <template #default="scope">
                    <el-descriptions title="服务器详情" border :column="3">
                        <el-descriptions-item label="在线服务器">{{ scope.row.actor.server }}</el-descriptions-item>
                        <el-descriptions-item label="在线玩家">{{ scope.row.actor.player }}</el-descriptions-item>

                        <el-descriptions-item label="内网总处理">{{ scope.row.dispatch.sum }}</el-descriptions-item>
                        <el-descriptions-item label="内网待处理">{{ scope.row.dispatch.wait }}
                        </el-descriptions-item>
                        <el-descriptions-item v-if="scope.row.redis" label="redis总处理">{{ scope.row.redis.sum }}
                        </el-descriptions-item>
                        <el-descriptions-item v-if="scope.row.redis" label="redis待处理">
                            {{ scope.row.redis.wait }}
                        </el-descriptions-item>

                        <el-descriptions-item v-if="scope.row.mongo" label="mongo总处理">{{ scope.row.mongo.sum }}
                        </el-descriptions-item>
                        <el-descriptions-item v-if="scope.row.mongo" label="mongo待处理">
                            {{ scope.row.mongo.wait }}
                        </el-descriptions-item>

                        <el-descriptions-item v-if="scope.row.web" label="web总处理">{{ scope.row.web.sum }}
                        </el-descriptions-item>
                        <el-descriptions-item v-if="scope.row.web" label="web待处理">{{ scope.row.web.wait }}
                        </el-descriptions-item>
                        <el-descriptions-item v-if="scope.row.web" label="web连接数">{{ scope.row.web.client }}
                        </el-descriptions-item>

                        <el-descriptions-item v-if="scope.row.outer" label="网关总处理消息">{{ scope.row.outer.sum }}
                        </el-descriptions-item>
                        <el-descriptions-item v-if="scope.row.outer" label="网关待处理消息">{{ scope.row.outer.wait }}
                        </el-descriptions-item>
                        <el-descriptions-item v-if="scope.row.lua" label="lua使用内存">{{ format_size(null, null, scope.row.lua) }}
                        </el-descriptions-item>
                    </el-descriptions>
                </template>
            </el-table-column>
            <el-table-column prop="id" align="center" width="100%" label="ID"></el-table-column>
            <el-table-column prop="name" align="center" width="100%" label="名字"></el-table-column>
            <el-table-column align="center" prop="time" label="启动时间" width="300%" :formatter="format_time"></el-table-column>
            <el-table-column align="center" label="cpu占用">
                <template #default="scope">{{ scope.row.cpu.toFixed(3) + "%" }}</template>
            </el-table-column>
            <el-table-column align="center" prop="use_memory" label="使用内存"
                             :formatter="format_size"></el-table-column>
            <el-table-column align="center" prop="max_memory" label="总内存" :formatter="format_size"></el-table-column>

            <el-table-column label="操作" align="center" width="200%">
                <template #default="scope">
                    <el-button type="primary" @click="onHotfixServer(scope.row.id)">热更</el-button>
                    <el-button type="danger" @click="onStopServer(scope.row.id)">关闭</el-button>
                </template>
            </el-table-column>
        </el-table>
    </div>
</template>

<script setup lang="ts" name="dashboard">
import {onActivated, onMounted, ref} from 'vue';
import {ElMessage, ElMessageBox} from "element-plus";
import {format_size, format_time} from "../api/format";
import {RequestAllList, RequestHotfix, RequestStopServer} from "../api/server";

interface ServerInfo {
    id: number,
    time: number,
    name: string,
    cpu: number,
    fps: number,
    ping: string,
    use_memory: number,
    max_memory: number,
    lua: number,
    log: { level: number, count: number }
    outer: { sum: number, wait: number },
    redis: { sum: number, wait: number },
    mongo: { sum: number, wait : number }
    dispatch: { sum: number, wait: number }
    listen : { rpc : string, gate : string, http : string }
    web: { sum: number, wait: number, client: number, pool: number },
    actor: { player: number, server: number }
}


const server_name = ref<string>("")
const server_list = ref<ServerInfo[]>([])
const server_name_list = ref<string[]>([])

const onStopServer = async (id: number) => {
    await ElMessageBox.confirm('确定关闭服务器吗？', '提示', {
        type: 'warning'
    });
    const response = await RequestStopServer(id)

    if (response.data.code == 0) {
        ElMessage.success("关闭服务器成功")
        return
    }
    ElMessage.success("关闭服务器失败")
}

const onHotfixServer = async (id: number) => {

    await ElMessageBox.confirm('确定热更服务器吗？', '提示', {
        type: 'warning'
    });

    RequestHotfix(id).then((response) => {
        if (response.data.code == 0) {
            ElMessage.success("服务器热更新成功")
            return
        }
        window.alert("服务器热更新失败")
    }).catch(() => {
        window.alert("服务器热更新失败")
    })
}

const GetServerList = () => {
    server_name_list.value.length = 0
    RequestAllList().then((response) => {
        const data = response.data.list as ServerInfo[]
        data.map(info => {
            info.fps = Math.floor(info.fps)
            info.time = info.time / 1000
            server_name_list.value.push(info.name)
        })
        if (server_name_list.value.length > 0) {
            server_name.value = server_name_list.value[0]
        }
        server_list.value = data;
        server_list.value.sort((a, b) => a.id - b.id);
    }).catch((res)=> {
        ElMessage.error(`请检查网络 : ${res.statusText}`)
    })
}

onMounted(() => GetServerList())
onActivated(() => {
    console.log("onActivated")
})

</script>

<style scoped>

.grid-cont-right {
    flex: 1;
    text-align: center;
    font-size: 40px;
    color: #999;
}

.user-info-cont div:first-child {
    font-size: 30px;
    color: #222;
}
</style>
