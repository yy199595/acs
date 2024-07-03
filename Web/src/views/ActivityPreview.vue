<template>
    <div>

        <el-descriptions border :column="2">
            <el-descriptions-item label="标题">{{ info.title }}</el-descriptions-item>
            <el-descriptions-item label="地址">{{ info.address }}</el-descriptions-item>
            <el-descriptions-item label="普通价">{{ "¥" + (info.price / 100).toFixed(2) }}</el-descriptions-item>
            <el-descriptions-item label="会员价">{{ "¥" + (info.vip_price / 100).toFixed(2) }}</el-descriptions-item>
            <el-descriptions-item label="开始时间">{{ new Date(info.start_time * 1000).toLocaleString() }}
            </el-descriptions-item>
            <el-descriptions-item label="结束时间">{{ new Date(info.stop_time * 1000).toLocaleString() }}
            </el-descriptions-item>
            <el-descriptions-item label="标签">{{ info.tag }}</el-descriptions-item>
        </el-descriptions>
        <div v-html="info.content" class="rich-text-container" style="overflow : auto"></div>
    </div>


</template>

<script>
export default {
    props: {
        info: {
            type: Object,
            required: true
        }
    }
}
</script>

<style scoped>
.carousel-item-container {
    display: flex;
    justify-content: center;
    align-items: center;
    height: 100vh; /* 设置图片容器的高度，可根据实际需求调整 */
}

.centered-image {
    max-width: 100%; /* 限制图片的最大宽度为容器的宽度 */
    max-height: 100%; /* 限制图片的最大高度为容器的高度 */
    width: auto;
    height: auto;
}
</style>
