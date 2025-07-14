<template>
    <div class="container">
        <el-table class="table" :data="serverList" border v-loading="loading">
            <el-table-column prop="id" align="center" width="100" label="ID"></el-table-column>
            <el-table-column prop="timeout" align="center" width="100" label="延时"></el-table-column>
            <el-table-column prop="name" align="center" width="180" label="名字"></el-table-column>
            <el-table-column align="center" prop="start_time" label="启动时间" width="200"></el-table-column>
            <el-table-column align="center" prop="cpu" label="CPU占用" width="120"></el-table-column>
            <el-table-column align="center" prop="use_memory" label="使用内存"></el-table-column>
            <el-table-column align="center" prop="max_memory" label="总内存"></el-table-column>

            <el-table-column label="操作" align="center" width="200">
                <template #default="scope">
                    <el-button type="primary" @click="handleHotfix(scope.row.id)" size="small">热更</el-button>
                    <el-button type="danger" @click="handleStop(scope.row.id)" size="small" :loading="isStopping[scope.row.id]">关闭</el-button>
                </template>
            </el-table-column>
        </el-table>
    </div>
</template>

<script setup>
import { ref, onMounted, nextTick } from 'vue'
import { ElMessage, ElMessageBox } from 'element-plus'
import { RequestAllList, RequestHotfix, RequestStopServer } from '../api/server'

// 响应式数据
const loading = ref(false)
const serverList = ref([])
const isStopping = ref({})

// 获取服务器列表
const fetchServerList = async () => {
    try {
        loading.value = true

        serverList.value = []
        const response = await RequestAllList()

        // 确保数据存在且格式正确
        const data = response.data.list || []
        for (let i = 0; i < data.length; i++) {
            const item = data[i]
            if(!item.error)
            {
                serverList.value.push(item)
            }
        }

        ElMessage.success('服务器列表加载成功')
    } catch (error) {
        console.error('获取服务器列表失败:', error)
        ElMessage.error(`获取服务器列表失败: ${error.message || '未知错误'}`)
    } finally {
        loading.value = false
    }
}

// 格式化时间函数
const formatTime = (timestamp) => {
    if (!timestamp) return ''
    const date = new Date(timestamp)
    return date.toLocaleString()
}

// 热更新服务器
const handleHotfix = async (id) => {
    try {
        await ElMessageBox.confirm('确定要热更新此服务器吗？', '确认', {
            type: 'warning'
        })

        const response = await RequestHotfix(id)

        if (response.data.code === 0) {
            ElMessage.success('热更新成功')
            await fetchServerList() // 更新列表
        } else {
            ElMessage.error(`热更新失败: ${response.data.message || '未知错误'}`)
        }
    } catch (error) {
        ElMessage.info('操作已取消')
    }
}

// 停止服务器
const handleStop = async (id) => {
    try {
        isStopping.value[id] = true

        await ElMessageBox.confirm('确定要停止此服务器吗？', '确认', {
            type: 'warning'
        })

        const response = await RequestStopServer(id)

        if (response.data.code === 0) {
            ElMessage.success('服务器已停止')
            await fetchServerList() // 更新列表
        } else {
            ElMessage.error(`停止服务器失败: ${response.data.message || '未知错误'}`)
        }
    } catch (error) {
        ElMessage.info('操作已取消')
    } finally {
        isStopping.value[id] = false
    }
}

// 组件挂载后加载数据
onMounted(async () => {
    await fetchServerList()

    // 确保DOM更新后执行
    nextTick(() => {
        console.log('表格已渲染')
    })
})
</script>

<style scoped>
.container {
    padding: 15px;
}

.table {
    margin-top: 15px;
}
</style>